#include <Input.h>

#include <DigitalInput.h>
#include <DomoticNode.h>
#include <domoticPi.h>
#include <exceptions.h>
#include <SerialInterface.h>

#include <stdexcept>
#include <wiringPi.h>

using namespace domotic_pi;

Input::Input(const std::string& id, int pinNumber) : 
	Pin(pinNumber), Module(id)
{
	if (pinNumber >= 0) {
		pinMode(pinNumber, INPUT);

		console->info("Input::ctor : pin %d set as input.", pinNumber);
	}	
}

Input::~Input() 
{
}

Input_ptr Input::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode, bool checkSchema)
{
	// If needed check that config schema is correct before performing initialization
	if (checkSchema && !json::hasValidSchema(config, DOMOTIC_PI_JSON_INPUT)) {
		console->error("Input::fromJson : json schema not valid for an Input.");
		throw domotic_pi_exception("Json configuration for domotic_pi::Input not valid.");
	}

	std::string id = config["id"].GetString();
	Input_ptr input = parentNode->getInput(id);
	if (input != nullptr) {
		console->warn("Input::from_json : input '%s' already loaded.", id.c_str());
		return input;
	}

	std::string type = config["type"].GetString();

	if (type == "digital") {
		input = std::make_shared<DigitalInput>(
			id,
			config["pin"].GetInt(),
			config["pud"].GetInt());
	}
	
	if (type == "serial") {

		// If serialInterface is an object, then the serial interface requested needs to be created
		bool create = config["serialInterface"].IsObject();

		SerialInterface_ptr si;

		if (create) {
			si = SerialInterface::from_json(config["serialInterface"], parentNode);
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

		// TODO: initialize serial input here

	}

	if (config.HasMember("name"))
		input->setName(config["name"].GetString());

	parentNode->addInput(input);

	console->info("Input::from_json : new %s input created with id '%s' on node '%s'.",
		type.c_str(), id.c_str(), parentNode->getID().c_str());

	return input;
}

Input_ptr Input::from_json(const std::string& jsonConfig, DomoticNode_ptr parentNode)
{
	// Convert given string in a json document
	rapidjson::Document config;
	if (config.Parse(jsonConfig.c_str()).HasParseError()) {
		console->error("Input::fromJson : given json configuration string could not be parsed correctly into a json document.");
		throw domotic_pi_exception("Could not parse given json string.");
	}

	// Call standard parser asking for a schema check too
	return Input::from_json(config, parentNode, true);
}