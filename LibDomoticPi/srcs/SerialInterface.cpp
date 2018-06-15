#include <SerialInterface.h>

#include <DomoticNode.h>
#include <domoticPi.h>
#include <exceptions.h>

#include <errno.h>
#include <wiringSerial.h>

using namespace domotic_pi;

SerialInterface::SerialInterface(const std::string& port, int baud, int txPin, int rxPin) :
	_port(port), _baudRate(baud), _pinTX(txPin), _pinRX(rxPin)
{
	_serial = serialOpen(port.c_str(), baud);

	if (_serial < 0) {
		console->error("SerialInterface::ctor : serial port couldn't be open: %s.", strerror(errno));
		throw domotic_pi_exception("Couldn't open serial port.");
	}

	console->info("SerialInterface::ctor : serial open on %s at %d baud.", _port, baud);
}

SerialInterface::~SerialInterface()
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lckrx(_rxOwn);
	std::unique_lock<std::mutex> lcktx(_txOwn);
#endif

	serialClose(_serial);

	console->info("SerialInterface::dtor : serial close on %s.", _port);
}

SerialInterface_ptr SerialInterface::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode, bool checkSchema)
{
	if (checkSchema && !json::hasValidSchema(config, DOMOTIC_PI_JSON_SERIAL_INTERFACE)) {
		console->error("SerialInterface::from_json : invalid json schema for a serial interface.");
		throw domotic_pi_exception("Json configuration for domotic_pi::SerialInterface not valid.");
	}

	std::string port = config["port"].GetString();

	SerialInterface_ptr si = parentNode->getSerialInterface(port);
	if (si != nullptr) {
		console->warn("SerialInterface::from_json : parsed serial interface '%s' is already loaded.", port.c_str());
		return si;
	}

	int txPin = config.HasMember("txPin") ? config["txPin"].GetInt() : -1;
	int rxPin = config.HasMember("rxPin") ? config["rxPin"].GetInt() : -1;

	si = std::make_shared<SerialInterface>(
		port,
		config["baud"].GetInt(),
		txPin,
		rxPin);

	parentNode->addSerialInterface(si);

	console->info("SerialInterface::from_json : new serial interface created from port '%s' on node '%s'.",
		port.c_str(), parentNode->getID().c_str());

	return si;
}

SerialInterface_ptr SerialInterface::from_json(const std::string& jsonConfig, DomoticNode_ptr parentNode)
{
	rapidjson::Document config;
	if (config.Parse(jsonConfig.c_str()).HasParseError()) {
		console->error("SerialInterface::from_json : could not parse given configuration string.");
		throw domotic_pi_exception("Could not parse given json srting.");
	}

	return SerialInterface::from_json(config, parentNode, true);
}

const std::string & SerialInterface::getPort() const
{
	return _port;
}

int SerialInterface::getBaudRate() const
{
	return _baudRate;
}

std::string SerialInterface::read()
{
	std::string message;
	int bytes = 0;

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_rxOwn);
#endif

	// Check if there is something to read
	if (!(bytes = serialDataAvail(_serial)))
		return message;

	if (bytes < 0) {
		console->error("SerialInterface::read : error during data availability check: %s.", strerror(errno));
		return message;
	}

	// Allocate read buffer
	char *buffer = new char[bytes];

	// Read all available bytes and store them in buffer
	for (int i = 0; i < bytes; i++)
		buffer[i] = serialGetchar(_serial);

	// Assign read buffer to message string
	message.assign(buffer);

	console->debug("SerialInterface::read : read message: '%s'.", message.c_str());

	return message;
}

void SerialInterface::write(const std::string & message)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_txOwn);
#endif

	serialPrintf(_serial, "%s", message.c_str());

	console->debug("SerialInterface::read : wrote message: '%s'.", message.c_str());
}

rapidjson::Document SerialInterface::to_json() const
{
	console->debug("SerialInterface::to_json : serializing serial interface '%s'.", 
		_port.c_str());

	rapidjson::Document serialInterface(rapidjson::kObjectType);

	rapidjson::Value port;
	port.SetString(_port.c_str(), serialInterface.GetAllocator());
	serialInterface.AddMember("port", port, serialInterface.GetAllocator());

	serialInterface.AddMember("baud", _baudRate, serialInterface.GetAllocator());

	if (_pinTX.getPin() >= 0)
		serialInterface.AddMember("txPin", _pinTX.getPin(), serialInterface.GetAllocator());

	if (_pinRX.getPin() >= 0)
		serialInterface.AddMember("rxPin", _pinRX.getPin(), serialInterface.GetAllocator());

	return serialInterface;
}