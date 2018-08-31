#include <InputFactory.h>

#include <domoticPi.h>
#include <exceptions.h>

using namespace domotic_pi;

#ifdef DOMOTIC_PI_THREAD_SAFE
std::mutex InputFactory::_inputInitMap;
#endif // DOMOTIC_PI_THREAD_SAFE

Input_ptr InputFactory::from_json(const rapidjson::Value& config,
	DomoticNode_ptr parentNode,
	bool checkSchema)
{
	// If needed check that config schema is correct before performing initialization
	if (checkSchema && !json::hasValidSchema(config, DOMOTIC_PI_JSON_INPUT)) {
		console->error("InputFactory::fromJson : json schema not valid for an Input.");
		throw domotic_pi_exception("Json configuration for domotic_pi::Input not valid.");
	}

	std::string id = config["id"].GetString();
	Input_ptr input = parentNode->getInput(id);
	if (input != nullptr) {
		console->warn("InputFactory::from_json : input '{}' already loaded.", id.c_str());
		return input;
	}

	std::string inputType = config["type"].GetString();
	input = _inputInitializers()[inputType](config, parentNode);

	if (config.HasMember("name"))
		input->setName(config["name"].GetString());

	parentNode->addInput(std::dynamic_pointer_cast<IInput>(input));

	console->info("InputFactory::from_json : new {} input created with id '{}' on node '{}'.",
		inputType.c_str(), id.c_str(), parentNode->getID().c_str());

	return input;
}

Input_ptr InputFactory::from_json(const std::string& jsonConfig,
	DomoticNode_ptr parentNode)
{
	// Convert given string in a json document
	rapidjson::Document config;
	if (config.Parse(jsonConfig.c_str()).HasParseError()) {
		console->error("InputFactory::fromJson : given json configuration string could not be parsed correctly into a json document.");
		throw domotic_pi_exception("Could not parse given json string.");
	}

	// Call standard parser asking for a schema check too
	return InputFactory::from_json(config, parentNode, true);
}

bool InputFactory::initializer_registration(
	const std::string& inputType,
	std::function<Input_ptr(const rapidjson::Value&, DomoticNode_ptr)> from_json)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_inputInitMap);
#endif // DOMOTIC_PI_THREAD_SAFE

	if (_inputInitializers().find(inputType) != _inputInitializers().end()) {
		console->error("InputFactory::initializer_registration : input type identifier '{}' already in use.");

		throw domotic_pi_exception("Input initializer duplicated");
	}

	if (!_inputInitializers().emplace(inputType, from_json).second) {
		throw domotic_pi_exception("Could not emplace initializer in factory map");
	}

	return true;
}

std::map<const std::string, std::function<Input_ptr(const rapidjson::Value&, DomoticNode_ptr)>> &InputFactory::_inputInitializers()
{
	static std::map<const std::string, std::function<Input_ptr(const rapidjson::Value&, DomoticNode_ptr)>> inputInitializers;
	return inputInitializers;
}