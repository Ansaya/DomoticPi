#include <domoticPi.h>

#include <domoticPiDefine.h>

#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <wiringPi.h>

std::shared_ptr<spdlog::logger> domotic_pi::console = spdlog::stdout_color_mt("domoticPi");

void domotic_pi::setConsole(std::shared_ptr<spdlog::logger> console)
{
	domotic_pi::console = console;
}

int domotic_pi::domoticPiInit()
{
	int retval = domotic_pi::json::SchemaProvider::load();
	if (retval) {
		console->error("domoticPiInit : could not load json schemas.\n");
		return retval;
	}

	retval = wiringPiSetup(WPI_MODE_PINS);

	if (retval)
		console->error("domoticPiInit : wiringPiSetup returned {} code.", retval);
	else
		console->info("domoticPiInit : domoticPi initialization succesful.");

	return retval;
}

std::map<std::string, std::shared_ptr<rapidjson::SchemaDocument>> 
domotic_pi::json::SchemaProvider::_schemas;

int domotic_pi::json::SchemaProvider::load()
{
	const char * syspath =
#include "../json-schema/syspath.json"
		;
	rapidjson::Document syspathDoc;
	if (syspathDoc.Parse(syspath).HasParseError()) {
		console->error("json::SchemaProvider::load : error found while loading json "
			"schema 'syspath.json'.");
		return -1;
	}
	_schemas["syspath.json"] = std::make_shared<rapidjson::SchemaDocument>(syspathDoc);

	const char * pinNumber =
#include "../json-schema/pinNumber.json"
		;
	rapidjson::Document pinNumberDoc;
	if (pinNumberDoc.Parse(pinNumber).HasParseError()) {
		console->error("json::SchemaProvider::load : error found while loading json "
			"schema 'pinNumber.json'.");
		return -1;
	}
	_schemas["pinNumber.json"] = 
		std::make_shared<rapidjson::SchemaDocument>(pinNumberDoc);

	const char * InterfaceType =
#include "../json-schema/InterfaceType.json"
		;
	rapidjson::Document InterfaceTypeDoc;
	if (InterfaceTypeDoc.Parse(InterfaceType).HasParseError()) {
		console->error("json::SchemaProvider::load : error found while loading json "
			"schema 'InterfaceType.json'.");
		return -1;
	}
	_schemas["InterfaceType.json"] =
		std::make_shared<rapidjson::SchemaDocument>(InterfaceTypeDoc);

	const char * MqttInterface =
#include "../json-schema/MqttInterface.json"
		;
	rapidjson::Document MqttInterfaceDoc;
	if (InterfaceTypeDoc.Parse(MqttInterface).HasParseError()) {
		console->error("json::SchemaProvider::load : error found while loading json "
			"schema 'MqttInterface.json'.");
		return -1;
	}
	_schemas["MqttInterface.json"] =
		std::make_shared<rapidjson::SchemaDocument>(MqttInterfaceDoc);

	const char * ioBinding =
#include "../json-schema/ioBinding.json"
		;
	rapidjson::Document ioBindingDoc;
	if (ioBindingDoc.Parse(ioBinding).HasParseError()) {
		console->error("json::SchemaProvider::load : error found while loading json "
			"schema 'ioBinding.json'.");
		return -1;
	}
	_schemas["ioBinding.json"] = std::make_shared<rapidjson::SchemaDocument>(ioBindingDoc);

	domotic_pi::json::SchemaProvider provider;

	const char * SerialInterface =
#include "../json-schema/SerialInterface.json"
		;
	rapidjson::Document SerialInterfaceDoc;
	if (SerialInterfaceDoc.Parse(SerialInterface).HasParseError()) {
		console->error("json::SchemaProvider::load : error found while loading json "
			"schema '" STR(DOMOTIC_PI_JSON_SERIAL_INTERFACE) "'.");
		return -1;
	}
	_schemas[DOMOTIC_PI_JSON_SERIAL_INTERFACE] =
		std::make_shared<rapidjson::SchemaDocument>(SerialInterfaceDoc, nullptr, 0, &provider);

	const char * Input =
#include "../json-schema/Input.json"
		;
	rapidjson::Document InputDoc;
	if (InputDoc.Parse(Input).HasParseError()) {
		console->error("json::SchemaProvider::load : error found while loading json "
			"schema '" STR(DOMOTIC_PI_JSON_INPUT) "'.");
		return -1;
	}
	_schemas[DOMOTIC_PI_JSON_INPUT] = 
		std::make_shared<rapidjson::SchemaDocument>(InputDoc, nullptr, 0, &provider);

	const char * Output =
#include "../json-schema/Output.json"
		;
	rapidjson::Document OutputDoc;
	if (OutputDoc.Parse(Output).HasParseError()) {
		console->error("json::SchemaProvider::load : error found while loading json "
			"schema '" STR(DOMOTIC_PI_JSON_OUTPUT) "'.");
		return -1;
	}
	_schemas[DOMOTIC_PI_JSON_OUTPUT] = 
		std::make_shared<rapidjson::SchemaDocument>(OutputDoc, nullptr, 0, &provider);

	const char * DomoticNode =
#include "../json-schema/DomoticNode.json"
		;
	rapidjson::Document DomoticNodeDoc;
	if (DomoticNodeDoc.Parse(DomoticNode).HasParseError()) {
		console->error("json::SchemaProvider::load : error found while loading json "
			"schema '" STR(DOMOTIC_PI_JSON_DOMOTIC_NODE) "'.");
		return -1;
	}
	_schemas[DOMOTIC_PI_JSON_DOMOTIC_NODE] = 
		std::make_shared<rapidjson::SchemaDocument>(DomoticNodeDoc, nullptr, 0, &provider);

	console->info("json::SchemaProvider::load : json schemas loaded correctly.");

	return 0;
}

bool domotic_pi::json::hasValidSchema(const rapidjson::Value& json, const char* schemaUri)
{
	const rapidjson::SchemaDocument* sd = domotic_pi::json::SchemaProvider::GetSchema(schemaUri);
	if (sd == nullptr) {
		domotic_pi::console->error("domotic_pi::json::hasValidSchema : json schema '{}' not found.", schemaUri);
		return false;
	}

	rapidjson::SchemaValidator validator(*sd);
	if (!json.Accept(validator)) {
		rapidjson::StringBuffer sb;
		validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);

		rapidjson::StringBuffer errorBuf;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(errorBuf);
		validator.GetError().Accept(writer);

		console->error("domotic_pi::json::hasValidSchema : invalid configuration file : "
			"\nInvalid shcema: {}\nInvalid keyword: {}\n{}",
			sb.GetString(), validator.GetInvalidSchemaKeyword(), errorBuf.GetString());

		sb.Clear();
		return false;
	}

	return true;
}

const rapidjson::SchemaDocument* domotic_pi::json::SchemaProvider::GetSchema(const char* uri)
{
	std::string fileName(uri);

	if (_schemas.find(fileName) != _schemas.end())
		return _schemas[fileName].get();

	console->warn("json::SchemaProvider::GetSchema : schema '{}' not found.", uri);

	return nullptr;
}

const rapidjson::SchemaDocument * 
domotic_pi::json::SchemaProvider::GetRemoteDocument(const char * uri, rapidjson::SizeType length)
{
	return domotic_pi::json::SchemaProvider::GetSchema(uri);
}
