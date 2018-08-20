#include <OutputFactory.h>

#include <DomoticNode.h>
#include <domoticPi.h>
#include <exceptions.h>
#include <IOutput.h>

using namespace domotic_pi;

#ifdef DOMOTIC_PI_THREAD_SAFE
std::mutex OutputFactory::_outputInitMap;
#endif // DOMOTIC_PI_THREAD_SAFE

std::map<std::string, std::function<Output_ptr(const rapidjson::Value&, DomoticNode_ptr)>> OutputFactory::_outputInitializers;

Output_ptr OutputFactory::from_json(
	const rapidjson::Value& config,
	DomoticNode_ptr parentNode,
	bool checkSchema)
{
	// If needed check that config schema is correct before performing initialization
	if (checkSchema && !json::hasValidSchema(config, DOMOTIC_PI_JSON_OUTPUT)) {
		console->error("OutputFactory::fromJson : json schema not valid for an Output.");
		throw domotic_pi_exception("Json configuration for domotic_pi::Output not valid.");
	}

	// Check if an output module with the same id has already been loaded
	std::string id = config["id"].GetString();
	Output_ptr output = parentNode->getOutput(id);
	if (output != nullptr) {
		console->warn("OutputFactory::from_json : output '{}' already loaded.", id.c_str());
		return output;
	}

	// Get requeired output type and call relative class initializer
	std::string outputType = config["type"].GetString();
	output = _outputInitializers[outputType](config, parentNode);

	if (config.HasMember("name")) {
		output->setName(config["name"].GetString());
	}

	parentNode->addOutput(output);

	console->info("OutputFactory::from_json : new {} output created with id '{}' on node '{}'.",
		outputType.c_str(), id.c_str(), parentNode->getID().c_str());

	return output;
}

Output_ptr OutputFactory::from_json(
	const std::string& jsonConfig,
	DomoticNode_ptr parentNode)
{
	rapidjson::Document config;
	if (config.Parse(jsonConfig.c_str()).HasParseError()) {
		console->error("OutputFactory::from_json : could not parse given json configuration to document.");
		throw domotic_pi_exception("Could not parse given json string.");
	}

	return OutputFactory::from_json(config, parentNode, true);
}

bool OutputFactory::initializer_registration(
	const std::string& outputType,
	std::function<Output_ptr(const rapidjson::Value&, DomoticNode_ptr)> from_json)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_outputInitMap);
#endif // DOMOTIC_PI_THREAD_SAFE

	if (_outputInitializers.find(outputType) != _outputInitializers.end()) {
		console->error("OutputFactory::initializer_registration : output type identifier '{}' already in use.");

		throw domotic_pi_exception("Output initializer duplicated");
	}

	_outputInitializers[outputType] = from_json;

	return true;
}