#include <SerialOutput.h>

#include <domoticPi.h>
#include <exceptions.h>
#include <SerialInterface.h>

using namespace domotic_pi;

const bool SerialOutput::_factoryRegistration =
	OutputFactory::initializer_registration("SerialOutput", SerialOutput::from_json);

SerialOutput::SerialOutput(const std::string& id, SerialInterface_ptr serial, int min_range, int max_range) : 
	IOutput(id, "SerialOutput"), _serial(serial), _range_min(min_range), _range_max(max_range)
{
	_value = min_range;

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	hap::Service_ptr lightService = std::make_shared<hap::Service>(hap::service_lightBulb);
	_ahkAccessory->addService(lightService);

	lightService->addCharacteristic(_nameInfo);

	_stateInfo = std::make_shared<hap::BoolCharacteristics>(hap::char_on, hap::permission_all);
	_stateInfo->Characteristics::setValue(std::to_string(false));
	_stateInfo->setValueChangeCB([this](bool oldValue, bool newValue, void* sender) {
		if (oldValue != newValue)
			setState(newValue ? ON : OFF);
	});
	lightService->addCharacteristic(_stateInfo);

	_valueInfo = std::make_shared<hap::IntCharacteristics>(hap::char_brightness, hap::permission_all, 
		_range_min, _range_max, 1, hap::unit_percentage);
	_valueInfo->hap::Characteristics::setValue(std::to_string(_value));
	_valueInfo->setValueChangeCB([this](int oldValue, int newValue, void* sender) {
		if (oldValue != newValue)
			setValue(newValue);
	});
	lightService->addCharacteristic(_valueInfo);
#endif
}

SerialOutput::~SerialOutput()
{
	setValue(_range_min);
}

void SerialOutput::setState(OutState newState)
{
	switch (newState) {
	case ON:
		setValue(_range_max);
		break;
	case OFF:
		setValue(_range_min);
		break;
	case TOGGLE:
		if (_value > _range_min)
			setValue(_range_min);
		else
			setValue(_range_max);
		break;
	default:
		break;
	}
}

void SerialOutput::setValue(int newValue)
{
	// Check given value range and adjust it if necessary
	if (newValue < _range_min)
		newValue = _range_min;

	if (newValue > _range_max)
		newValue = _range_max;

	// Build serial command string
	std::string cmd(getID());
	cmd.append(std::to_string(newValue));

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_valueLock);
#endif

	// Send command through serial interface
	_serial->write(cmd);

	_value = newValue;

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_stateInfo->hap::Characteristics::setValue(std::to_string(_value != _range_min));
	_valueInfo->hap::Characteristics::setValue(std::to_string(_value));
#endif

	console->info("SerialOutput::setValue : output '{}' set to '{}'.", getID(), _value);
}

std::shared_ptr<SerialOutput> SerialOutput::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	// If serialInterface is an object, then the serial interface requested needs to be created
	bool create = config["serialInterface"].IsObject();

	SerialInterface_ptr si;

	if (create) {
		// TODO : initialize the interface through the InterfaceFactory
		si = SerialInterface::from_json(config["serialInterface"], parentNode, false);
	}
	else {
		// Get required serial port name
		std::string port = config["serialInterface"].GetString();

		si = parentNode->getSerialInterface(port);

		if (si == nullptr) {
			console->error("SerialOutput::fromJson : requested serial interface '{}' is not present on node '{}'.",
				port.c_str(), parentNode->getID().c_str());
			throw domotic_pi_exception("Required serial port not found");
		}
	}

	std::shared_ptr<SerialOutput> output = std::make_shared<SerialOutput>(
		config["id"].GetString(),
		si,
		config["range_min"].GetInt(),
		config["range_max"].GetInt());

	return output;
}

rapidjson::Document SerialOutput::to_json() const
{
	rapidjson::Document output = IOutput::to_json();

	console->debug("SerialOutput::to_json : serializing output '{}'.", _id.c_str());

	output.AddMember("type", "SerialOutput", output.GetAllocator());

	rapidjson::Value serialInterface;
	serialInterface.SetString(_serial->getPort().c_str(), output.GetAllocator());
	output.AddMember("serialInterface", serialInterface, output.GetAllocator());

	output.AddMember("range_min", _range_min, output.GetAllocator());
	output.AddMember("range_max", _range_max, output.GetAllocator());

	return output;
}