#include <DomoticNode.h>

#include <domoticPi.h>
#include <exceptions.h>
#include <IInput.h>
#include <InputFactory.h>
#include <IOutput.h>
#include <OutputFactory.h>
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

	// Load available comm interfaces
	if (config.HasMember("comms")) {
		console->info("DomoticNode::from_json : loading comm interfaces for node '{}'.",
			domoticNode->getID().c_str());

		for (auto& it : config["comms"].GetArray()) {
			CommFactory::from_json(it, domoticNode);
		}
	}

	// Load available outputs
	if (config.HasMember("outputs")) {
		console->info("DomoticNode::from_json : loading outputs for node '{}'.",
			domoticNode->getID().c_str());

		for (auto& it : config["outputs"].GetArray()) {
			OutputFactory::from_json(it, domoticNode);
		}
	}

	// Load available programmed events
	if (config.HasMember("programmedEvents")) {
		console->info("DomoticNode::from_json : loading programmed events for node '{}'.",
			domoticNode->getID().c_str());

		for (auto& it : config["programmedEvents"].GetArray()) {
			ProgrammedEvent::from_json(it, domoticNode);
		}
	}

	// Load available inputs
	if (config.HasMember("inputs")) {
		console->info("DomoticNode::from_json : loading inputs for node '{}'.",
			domoticNode->getID().c_str());

		for (auto& it : config["inputs"].GetArray()) {
			InputFactory::from_json(it, domoticNode);
		}
	}

	// Set node name
	if (config.HasMember("name"))
		domoticNode->setName(config["name"].GetString());

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	// Enable Apple HomeKit if required
	if (config.HasMember("homekit")) {
		const rapidjson::Value& homekitConfig = config["homekit"];
		domoticNode->enableHAP(
			homekitConfig["password"].GetString(),
			homekitConfig.HasMember("name") ? homekitConfig["name"].GetString() : domoticNode->_name);
	}
#endif // DOMOTIC_PI_APPLE_HOMEKIT

	console->info("DomoticNode::from_json : new domotic node '{}' loaded succesfully.", 
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
	std::unique_lock<std::shared_mutex> lock(_nameLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	_name = name;
}

std::string DomoticNode::getName() const
{
	return _name;
}

Input_ptr DomoticNode::getInput(const std::string& id) const
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::shared_lock<std::shared_mutex> lock(_inputsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

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
	std::unique_lock<std::shared_mutex> lock(_inputsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	// Search for conflicting id in present inputs
	auto it = std::find_if(_inputs.begin(), _inputs.end(),
		[input](const Input_ptr& i) { return input->getID() == i->getID(); });

	// If conflict exists return immediately
	if (it != _inputs.end())
		return false;

	_inputs.push_back(input);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	if (input->hasAHKAccessory()) {
		hap::AccessorySet::getInstance().addAccessory(input->getAHKAccessory());
	}
#endif

	return true;
}

void DomoticNode::removeInput(const std::string & inputId)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::shared_mutex> lock(_inputsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	auto input = std::find_if(_inputs.begin(), _inputs.end(),
		[inputId](const Input_ptr& i) { return inputId == i->getID(); });

	if (input != _inputs.end()) {
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		if ((*input)->hasAHKAccessory()) {
			hap::AccessorySet::getInstance().removeAccessory((*input)->getAHKAccessory());
		}
#endif

		_inputs.erase(input);
	}
}

Output_ptr DomoticNode::getOutput(const std::string& id) const
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::shared_lock<std::shared_mutex> lock(_outputsLock);
#endif // DOMOTIC_PI_THREAD_SAFE


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
	std::unique_lock<std::shared_mutex> lock(_outputsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	// Search for conflicting id in present outputs
	auto it = std::find_if(_outputs.begin(), _outputs.end(),
		[output](const Output_ptr& o) { return output->getID() == o->getID(); });

	// If conflict exists return immediately
	if (it != _outputs.end())
		return false;

	_outputs.push_back(output);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	if(output->hasAHKAccessory()) {
		hap::AccessorySet::getInstance().addAccessory(output->getAHKAccessory());
	}
#endif

	return true;
}

void DomoticNode::removeOutput(const std::string & outputId)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::shared_mutex> lock(_outputsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	auto output = std::find_if(_outputs.begin(), _outputs.end(),
		[outputId](const Output_ptr& o) { return outputId == o->getID(); });

	if (output != _outputs.end()) {
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		if ((*output)->hasAHKAccessory()) {
			hap::AccessorySet::getInstance().removeAccessory((*output)->getAHKAccessory());
		}
#endif
		_outputs.erase(output);
	}
}

Comm_ptr DomoticNode::getComm(const std::string& id) const
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::shared_lock<std::shared_mutex> lock(_commsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	// Search for port name in already present serial interfaces
	auto it = std::find_if(_comms.begin(), _comms.end(),
		[id](const Comm_ptr& si) {
		return id == si->getID();
	});

	// If port exists return found serial interface
	if (it != _comms.end())
		return *it;

	return nullptr;
}

const std::vector<Comm_ptr>& DomoticNode::getComms() const
{
	return _comms;
}

bool DomoticNode::addComm(Comm_ptr comm)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::shared_mutex> lock(_commsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	// Search for port name in already present serial interfaces
	auto it = std::find_if(_comms.begin(), _comms.end(),
		[comm](const Comm_ptr& si) {
		return comm->getID() == si->getID();
	});

	// If conflict exists return immediately
	if (it != _comms.end())
		return false;

	_comms.push_back(comm);

	return true;
}

void DomoticNode::removeComm(const std::string & id)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::shared_mutex> lock(_commsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	std::remove_if(_comms.begin(), _comms.end(), 
		[id](const Comm_ptr& comm) {
			return id == comm->getID();
		});
}

ProgrammedEvent_ptr DomoticNode::getProgrammedEvent(const std::string& id) const
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::shared_lock<std::shared_mutex> lock(_programmedEventsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	// Search for event id in already present programmed events
	auto it = std::find_if(_programmedEvents.begin(), _programmedEvents.end(), 
		[&](const ProgrammedEvent_ptr &evt) {
			return evt->getID() == id;
		});

	if (it != _programmedEvents.end()) {
		return *it;
	}

	return nullptr;
}

const std::vector<ProgrammedEvent_ptr> &DomoticNode::getProgrammedEvents() const
{
	return _programmedEvents;
}

bool DomoticNode::addProgrammedEvent(ProgrammedEvent_ptr programmedEvent)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::shared_mutex> lock(_programmedEventsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	// Search for event id in already present programmed events
	auto it = std::find_if(_programmedEvents.begin(), _programmedEvents.end(),
		[&](const ProgrammedEvent_ptr &evt) {
		return evt->getID() == programmedEvent->getID();
	});

	// If conflict exists return immediately
	if (it != _programmedEvents.end()) {
		return false;
	}

	_programmedEvents.push_back(programmedEvent);

	return true;
}

void DomoticNode::removeProgrammedEvent(const std::string& id)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::shared_mutex> lock(_programmedEventsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	auto it = std::remove_if(_programmedEvents.begin(), _programmedEvents.end(),
		[&](const ProgrammedEvent_ptr &evt) {
		return evt->getID().compare(id) == 0;
	});
}

rapidjson::Document DomoticNode::to_json() const
{
	console->debug("DomoticNode::to_json : serializing node '{}'.", _id.c_str());

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

	// Populate and set node comm interfaces
	rapidjson::Value comms(rapidjson::kArrayType);
	for (auto& it : _comms)
		comms.PushBack(it->to_json(), domoticNode.GetAllocator());
	domoticNode.AddMember("comms", comms, domoticNode.GetAllocator());

	// Populate and set node programmed events
	rapidjson::Value programmedEvents(rapidjson::kArrayType);
	for (auto& it : _programmedEvents) {
		programmedEvents.PushBack(it->to_json(), domoticNode.GetAllocator());
	}
	domoticNode.AddMember("programmedEvents", programmedEvents, domoticNode.GetAllocator());

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	// Set node homekit information
	rapidjson::Value homekit(rapidjson::kObjectType);

	rapidjson::Value homekitName;
	homekitName.SetString(hap::net::HAPService::getInstance().getServiceName().c_str(), domoticNode.GetAllocator());
	homekit.AddMember("name", homekitName, domoticNode.GetAllocator());

	rapidjson::Value homekitPassword;
	homekitPassword.SetString(hap::net::HAPService::getInstance().getServicePassword().c_str(), domoticNode.GetAllocator());
	homekit.AddMember("password", homekitPassword, domoticNode.GetAllocator());

	domoticNode.AddMember("homekit", homekit, domoticNode.GetAllocator());
#endif // DOMOTIC_PI_APPLE_HOMEKIT

	return domoticNode;
}

#ifdef DOMOTIC_PI_APPLE_HOMEKIT

bool DomoticNode::enableHAP(const std::string& password, const std::string& hapName)
{
	return hap::net::HAPService::getInstance()
		.setupAndListen(hapName.empty() ? _id : hapName, password);
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