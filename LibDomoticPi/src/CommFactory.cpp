#include <CommFactory.h>

#include <DomoticNode.h>
#include <domoticPi.h>
#include <exceptions.h>

using namespace domotic_pi;

#ifdef DOMOTIC_PI_THREAD_SAFE
std::mutex CommFactory::_commInitMap;
#endif // DOMOTIC_PI_THREAD_SAFE

Comm_ptr CommFactory::from_json(const rapidjson::Value& config,
	DomoticNode_ptr parentNode,
	bool checkSchema)
{
	// If needed check that config schema is correct before performing initialization
	if (checkSchema && !json::hasValidSchema(config, DOMOTIC_PI_JSON_COMM)) {
		console->error("CommFactory::fromJson : json schema not valid for a Comm.");
		throw domotic_pi_exception("Json configuration for domotic_pi::IComm not valid.");
	}

	std::string id = config["id"].GetString();
	Comm_ptr comm = parentNode->getComm(id);
	if (comm != nullptr) {
		console->warn("CommFactory::from_json : comm '{}' already loaded.", id.c_str());
		return comm;
	}

	std::string commType = config["type"].GetString();
	comm = _commInitializers()[commType](config, parentNode);

	if (config.HasMember("name"))
		comm->setName(config["name"].GetString());

	parentNode->addComm(std::dynamic_pointer_cast<IComm>(comm));

	console->info("CommFactory::from_json : new {} comm created with id '{}' on node '{}'.",
		commType.c_str(), id.c_str(), parentNode->getID().c_str());

	return comm;
}

Comm_ptr CommFactory::from_json(const std::string& jsonConfig,
	DomoticNode_ptr parentNode)
{
	// Convert given string in a json document
	rapidjson::Document config;
	if (config.Parse(jsonConfig.c_str()).HasParseError()) {
		console->error("CommFactory::fromJson : given json configuration string could not be parsed correctly into a json document.");
		throw domotic_pi_exception("Could not parse given json string.");
	}

	// Call standard parser asking for a schema check too
	return CommFactory::from_json(config, parentNode, true);
}

bool CommFactory::initializer_registration(
	const std::string& commType,
	std::function<Comm_ptr(const rapidjson::Value&, DomoticNode_ptr)> from_json)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_commInitMap);
#endif // DOMOTIC_PI_THREAD_SAFE

	if (_commInitializers().find(commType) != _commInitializers().end()) {
		console->error("CommFactory::initializer_registration : comm type identifier '{}' already in use.");

		throw domotic_pi_exception("Comm initializer duplicated");
	}

	if (!_commInitializers().emplace(commType, from_json).second) {
		throw domotic_pi_exception("Could not emplace initializer in factory map");
	}

	return true;
}

std::map<const std::string, std::function<Comm_ptr(const rapidjson::Value&, DomoticNode_ptr)>> &CommFactory::_commInitializers()
{
	static std::map<const std::string, std::function<Comm_ptr(const rapidjson::Value&, DomoticNode_ptr)>> commInitializers;
	return commInitializers;
}