#include <DomoticNode.h>

#include <domoticPi.h>
#include <exceptions.h>
#include <Input.h>
#include <Output.h>
#include <SerialInterface.h>

#include <algorithm>
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
#include <hap/libHAP.h>
#endif // DOMOTIC_PI_APPLE_HOMEKIT

using namespace domotic_pi;

DomoticNode::DomoticNode(const std::string& id) : _id(id)
{
}

DomoticNode::~DomoticNode()
{
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	if (isHAPEnabled())
		disableHAP();
#endif
}

DomoticNode_ptr DomoticNode::from_json(const rapidjson::Value& config, bool checkSchema)
{
	if (checkSchema && !json::hasValidSchema(config, DOMOTIC_PI_JSON_DOMOTIC_NODE)) {
		console->error("DomoticNode::fromJson : json schema not valid for an Output.");
		throw domotic_pi_exception("Json configuration for domotic_pi::DomoticNode not valid.");
	}

	DomoticNode_ptr domoticNode = std::make_shared<DomoticNode>(config["id"].GetString());

	// Load available serial interface
	if (config.HasMember("serialInterfaces")) {
		console->info("DomoticNode::from_json : loading serial interfaces for node '%s'.",
			domoticNode->getID().c_str());

		const rapidjson::Value::ConstArray& serialInterfaces = config["serialInterfaces"].GetArray();
		for (auto& it : serialInterfaces) {
			SerialInterface::from_json(it, domoticNode);
		}
	}

	// Load available inputs
	if (config.HasMember("inputs")) {
		console->info("DomoticNode::from_json : loading inputs for node '%s'.",
			domoticNode->getID().c_str());

		const rapidjson::Value::ConstArray& inputs = config["inputs"].GetArray();
		for (auto& it : inputs) {
			Input::from_json(it, domoticNode);
		}
	}

	// Load available outputs
	if (config.HasMember("outputs")) {
		console->info("DomoticNode::from_json : loading outputs for node '%s'.",
			domoticNode->getID().c_str());

		const rapidjson::Value::ConstArray& outputs = config["outputs"].GetArray();
		for (auto& it : outputs) {
			Output::from_json(it, domoticNode);
		}
	}

	// Load input/output bindings
	if (config.HasMember("bindings")) {
		console->info("DomoticNode::from_json : loading I/O bindings for node '%s'.",
			domoticNode->getID().c_str());

		for (auto& it : config["bindings"].GetArray()) {
			const std::string inputId = it["inputId"].GetString();
			const std::string outputId = it["outputId"].GetString();
			const int isr_mode = it["inputState"].GetInt() + 1;
			const OutState outputAction = (OutState)it["outputAction"].GetInt();

			Input_ptr input = domoticNode->getInput(inputId);
			if (input == nullptr) {
				console->warn("DomoticNode::from_json : input '%s' not found for binding.",
					inputId);
				continue;
			}

			Output_ptr output = domoticNode->getOutput(outputId);
			if (output == nullptr) {
				console->warn("DomoticNode::from_json : output '%s' not found for binding.",
					outputId);
				continue;
			}

			// Weak reference is passed to input ISR to avoid nullptr 
			// in case the output is deleted afterwards
			std::weak_ptr<Output> weakOut = output;
			
			input->addISRCall([weakOut, outputAction]() {
				if (auto effectiveOut = weakOut.lock()) {
					effectiveOut->setState(outputAction);
				}
			}, isr_mode);

			console->info("DomoticNode::from_json : output '%s' bounded to input '%s'.", 
				outputId.c_str(), inputId.c_str());
		}
	}

	if (config.HasMember("name"))
		domoticNode->setName(config["name"].GetString());

	console->info("DomoticNode::from_json : new domotic node '%s' loaded succesfully.", 
		domoticNode->getID().c_str());

	return domoticNode;
}

DomoticNode_ptr DomoticNode::from_json(const std::string& jsonConfig)
{
	rapidjson::Document config;
	if (config.Parse(jsonConfig.c_str()).HasParseError()) {
		console->error("DomoticNode::from_json : could not parse given json configuration to document.");
		throw domotic_pi_exception("Could not parse given json string.");
	}

	return DomoticNode::from_json(config, true);
}

const std::string& DomoticNode::getID() const
{
	return _id;
}

void DomoticNode::setName(const std::string & name)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_nameLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	_name = name;
}

const std::string & DomoticNode::getName() const
{
	return _name;
}

Input_ptr DomoticNode::getInput(const std::string& id) const
{
	// Search for required id in present inputs
	auto it = std::find_if(_inputs.begin(), _inputs.end(),
		[id](const Input_ptr& i) { return id == i->getID(); });

	// If id exists return found input
	if (it != _inputs.end())
		return *it;

	return nullptr;
}

const std::vector<Input_ptr>& DomoticNode::getInputs() const
{
	return _inputs;
}

bool DomoticNode::addInput(Input_ptr input)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_inputsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	// If conflict exists return immediately
	if (getInput(input->getID()) != nullptr)
		return false;

	_inputs.push_back(input);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	hap::AccessorySet::getInstance().addAccessory(input->getAHKAccessory());
#endif

	return true;
}

void DomoticNode::removeInput(const std::string & inputId)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_inputsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	auto input = std::find_if(_inputs.begin(), _inputs.end(),
		[inputId](const Input_ptr& i) { return inputId == i->getID(); });

	if (input != _inputs.end()) {
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		hap::AccessorySet::getInstance().removeAccessory((*input)->getAHKAccessory());
#endif

		_inputs.erase(input);
	}
}

Output_ptr DomoticNode::getOutput(const std::string& id) const
{
	// Search for required id in present outputs
	auto it = std::find_if(_outputs.begin(), _outputs.end(),
		[id](const Output_ptr& o) { return id == o->getID(); });

	// If id exists return found output
	if (it != _outputs.end())
		return *it;

	return nullptr;
}

const std::vector<Output_ptr>& DomoticNode::getOutputs() const
{
	return _outputs;
}

bool DomoticNode::addOutput(Output_ptr output)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_outputsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	// If conflict exists return immediately
	if (getOutput(output->getID()) != nullptr)
		return false;

	_outputs.push_back(output);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	hap::AccessorySet::getInstance().addAccessory(output->getAHKAccessory());
#endif

	return true;
}

void DomoticNode::removeOutput(const std::string & outputId)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_outputsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	auto output = std::find_if(_outputs.begin(), _outputs.end(),
		[outputId](const Output_ptr& o) { return outputId == o->getID(); });

	if (output != _outputs.end()) {
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		hap::AccessorySet::getInstance().removeAccessory((*output)->getAHKAccessory());
#endif
		_outputs.erase(output);
	}
}

SerialInterface_ptr DomoticNode::getSerialInterface(const std::string& port) const
{
	// Search for port name in already present serial interfaces
	auto it = std::find_if(_serialPorts.begin(), _serialPorts.end(),
		[port](const SerialInterface_ptr& si) {
		return port == si->getPort();
	});

	// If port exists return found serial interface
	if (it != _serialPorts.end())
		return *it;

	return nullptr;
}

const std::vector<SerialInterface_ptr>& DomoticNode::getSerialInterfaces() const
{
	return _serialPorts;
}

bool DomoticNode::addSerialInterface(SerialInterface_ptr serialInterface)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_serialInterfacesLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	// If conflict exists return immediately
	if (getSerialInterface(serialInterface->getPort()) != nullptr)
		return false;

	_serialPorts.push_back(serialInterface);

	return true;
}

void DomoticNode::removeSerialInterface(const std::string & serialInterfacePort)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_serialInterfacesLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	std::remove_if(_serialPorts.begin(), _serialPorts.end(), 
		[serialInterfacePort](const SerialInterface_ptr& si) {
			return serialInterfacePort == si->getPort();
		});
}

rapidjson::Document DomoticNode::to_json() const
{
	console->debug("DomoticNode::to_json : serializing node '%s'.", _id.c_str());

	rapidjson::Document domoticNode(rapidjson::kObjectType);

	// Set node id
	rapidjson::Value id;
	id.SetString(_id.c_str(), domoticNode.GetAllocator());
	domoticNode.AddMember("id", id, domoticNode.GetAllocator());

	// Set node name
	rapidjson::Value name;
	name.SetString(_name.c_str(), domoticNode.GetAllocator());
	domoticNode.AddMember("name", name, domoticNode.GetAllocator());

	// Populate and set node inputs
	rapidjson::Value inputs(rapidjson::kArrayType);
	for (auto& it : _inputs)
		inputs.PushBack(it->to_json(), domoticNode.GetAllocator());
	domoticNode.AddMember("inputs", inputs, domoticNode.GetAllocator());

	// Populate and set node outputs
	rapidjson::Value outputs(rapidjson::kArrayType);
	for (auto& it : _outputs)
		outputs.PushBack(it->to_json(), domoticNode.GetAllocator());
	domoticNode.AddMember("outputs", outputs, domoticNode.GetAllocator());

	// Populate and set node serial interfaces
	rapidjson::Value serialInterfaces(rapidjson::kArrayType);
	for (auto& it : _serialPorts)
		serialInterfaces.PushBack(it->to_json(), domoticNode.GetAllocator());
	domoticNode.AddMember("serialInterfaces", serialInterfaces, domoticNode.GetAllocator());

	return domoticNode;
}

#ifdef DOMOTIC_PI_APPLE_HOMEKIT

bool DomoticNode::enableHAP()
{
	return hap::net::HAPService::getInstance()
		.setupAndListen(hap::deviceType_bridge);
}

void DomoticNode::disableHAP()
{
	hap::net::HAPService::getInstance().stop();
}

bool DomoticNode::isHAPEnabled() const
{
	return hap::net::HAPService::getInstance().isRunning();
}

#endif