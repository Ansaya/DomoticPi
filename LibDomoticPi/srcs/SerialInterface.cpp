#include <SerialInterface.h>

#include <DomoticNode.h>
#include <domoticPi.h>
#include <exceptions.h>

#include <errno.h>
#include <wiringSerial.h>

using namespace domotic_pi;

const bool SerialInterface::_factoryRegistration = 
	CommFactory::initializer_registration("SerialInterface", SerialInterface::from_json);

SerialInterface::SerialInterface(
	const std::string& id, 
	const std::string& port, 
	int baud, 
	int txPin, 
	int rxPin) 
	: IComm(id, "SerialInterface"), _port(port), _baudRate(baud), _pinTX(txPin), _pinRX(rxPin)
{
	_serial = serialOpen(port.c_str(), baud);

	if (_serial < 0) {
		console->error("SerialInterface::ctor : serial port couldn't be open: {}.", strerror(errno));
		throw domotic_pi_exception("Couldn't open serial port.");
	}

	console->info("SerialInterface::ctor : serial open on {} at {} baud.", _port, baud);
}

SerialInterface::~SerialInterface()
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lckrx(_rxOwn);
	std::unique_lock<std::mutex> lcktx(_txOwn);
#endif

	serialClose(_serial);

	console->info("SerialInterface::dtor : serial close on {}.", _port);
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
		console->error("SerialInterface::read : error during data availability check: {}.", strerror(errno));
		return message;
	}

	// Allocate read buffer
	char *buffer = new char[bytes];

	// Read all available bytes and store them in buffer
	for (int i = 0; i < bytes; i++)
		buffer[i] = (char)serialGetchar(_serial);

	// Assign read buffer to message string
	message.assign(buffer);

	console->debug("SerialInterface::read : read message: '{}'.", message.c_str());

	return message;
}

void SerialInterface::write(const std::string & message)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_txOwn);
#endif

	serialPrintf(_serial, "{}", message.c_str());

	console->debug("SerialInterface::read : wrote message: '{}'.", message.c_str());
}

std::shared_ptr<SerialInterface> SerialInterface::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	int txPin = config.HasMember("serialTxPin") ? config["serialTxPin"].GetInt() : -1;
	int rxPin = config.HasMember("serialRxPin") ? config["serialRxPin"].GetInt() : -1;

	return std::make_shared<SerialInterface>(
		config["id"].GetString(),
		config["serialPort"].GetString(),
		config["serialBaud"].GetInt(),
		txPin,
		rxPin);
}

rapidjson::Document SerialInterface::to_json() const
{
	console->debug("SerialInterface::to_json : serializing serial comm '{}'.", 
		_port.c_str());

	rapidjson::Document serialInterface = IComm::to_json();

	rapidjson::Value port;
	port.SetString(_port.c_str(), serialInterface.GetAllocator());
	serialInterface.AddMember("serialPort", port, serialInterface.GetAllocator());

	serialInterface.AddMember("serialBaud", _baudRate, serialInterface.GetAllocator());

	if (_pinTX.getPin() >= 0)
		serialInterface.AddMember("serialTxPin", _pinTX.getPin(), serialInterface.GetAllocator());

	if (_pinRX.getPin() >= 0)
		serialInterface.AddMember("serialRxPin", _pinRX.getPin(), serialInterface.GetAllocator());

	return serialInterface;
}