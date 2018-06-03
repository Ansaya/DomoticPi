#include "Output.h"

#include "DigitalOutput.h"
#include "DomoticNode.h"
#include "domoticPi.h"
#include "exceptions.h"
#include "SerialInterface.h"
#include "SerialOutput.h"

#include <wiringPi.h>

using namespace domotic_pi;

Output::Output(const std::string& id, int pinNumber) : 
	Pin(pinNumber), Module(id)
{
}

Output::~Output()
{
}

Output_ptr Output::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode, bool checkSchema)
{
	// If needed check that config schema is correct before performing initialization
	if (checkSchema && !json::hasValidSchema(config, DOMOTIC_PI_JSON_OUTPUT)) {
		console->error("Output::fromJson : json schema not valid for an Output.");
		throw domotic_pi_exception("Json configuration for domotic_pi::Output not valid.");
	}

	std::string id = config["id"].GetString();
	Output_ptr output = parentNode->getOutput(id);
	if (output != nullptr) {
		console->warn("Output::from_json : output '%s' already loaded.", id.c_str());
		return output;
	}

	std::string type = config["type"].GetString();
	if (type == "digital") {
		output = std::make_shared<DigitalOutput>(
			config["id"].GetString(),
			config["pin"].GetInt());
	}

	if (type == "serial") {

		// If serialInterface is an object, then the serial interface requested needs to be created
		bool create = config["serialInterface"].IsObject();

		SerialInterface_ptr si;
		
		if (create) {
			si = SerialInterface::from_json(config["serialInterface"], parentNode, false);
		}
		else {
			// Get required serial port name
			std::string port = config["serialInterface"].GetString();

			si = parentNode->getSerialInterface(port);

			if (si == nullptr) {
				console->error("Output::fromJson : requested serial interface '%s' is not present on node '%s'.",
					port.c_str(), parentNode->getID().c_str());
				throw domotic_pi_exception("Required serial port not found");
			}
		}

		output = std::make_shared<SerialOutput>(
			config["id"].GetString(),
			si,
			config["range_min"].GetInt(),
			config["range_max"].GetInt());
	}

	if (config.HasMember("name"))
		output->setName(config["name"].GetString());

	parentNode->addOutput(output);

	console->info("Output::from_json : new %s output created with id '%s' on node '%s'.", 
		type.c_str(), id.c_str(), parentNode->getID().c_str());

	return output;
}

Output_ptr Output::from_json(const std::string& jsonConfig, DomoticNode_ptr parentNode)
{
	rapidjson::Document config;
	if (config.Parse(jsonConfig.c_str()).HasParseError()) {
		console->error("Output::from_json : could not parse given json configuration to document.");
		throw domotic_pi_exception("Could not parse given json string.");
	}

	return Output::from_json(config, parentNode, true);
}

int Output::getValue() const 
{
	return _value;
}