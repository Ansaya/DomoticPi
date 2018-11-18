#include <DigitalSwitch.h>

#include <domoticPi.h>

#include <stdexcept>
#include <wiringPi.h>

using namespace domotic_pi;

const bool DigitalSwitch::_factoryRegistration =
	OutputFactory::initializer_registration("DigitalSwitch", DigitalSwitch::from_json);

DigitalSwitch::DigitalSwitch(const std::string& id, int pinNumber) 
	: Pin(pinNumber), IOutput(id)
{
	if (pinNumber < 0) {
		console->error("DigitalSwitch::ctor : pin number must be a valid pin for a digital output.");
		throw std::out_of_range("Pin number must be between 0 and " STR(DOMOTIC_PI_MAX_PIN) ".");
	}

	pinMode(pinNumber, OUTPUT);
	console->info("DigitalSwitch::ctor : pin {} set to OUTPUT mode.", pinNumber);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_ahkAccessory = std::make_shared<hap::Accessory>();

	_ahkAccessory->addInfoService(getName(), DOMOTIC_PI_APPLE_HOMEKIT_MANUFACTURER,
		typeid(DigitalSwitch).name(), _id, "1.0.0", [](bool oldValue, bool newValue, void* sender) {});

	_ahkAccessory->addSwitchService(&_stateInfo, &_nameInfo);

	_nameInfo->setValue(getName());

	_stateInfo->setValue(_value);
	_stateInfo->setValueChangeCB([this](bool oldValue, bool newValue, void* sender) {
		if (oldValue != newValue) {
			setState(newValue ? ON : OFF);
		}
	});
#endif
}

DigitalSwitch::~DigitalSwitch()
{
}

void DigitalSwitch::setState(OutState newState)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_valueLock);
#endif

	_value = newState == TOGGLE ? !_value : newState;

	digitalWrite(getPin(), _value);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_stateInfo->setValue(_value);
#endif

	console->info("DigitalSwitch::setState : output '{}' set to '{}'.", getID(), _value);
}

void DigitalSwitch::setValue(int newValue)
{
	if (newValue < 0)
		newValue = 0;

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_valueLock);
#endif

	_value = newValue;

	digitalWrite(getPin(), _value);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_stateInfo->setValue(_value);
#endif

	console->info("DigitalSwitch::setValue : output '{}' set to '{}'.", getID(), _value);
}

std::shared_ptr<DigitalSwitch> DigitalSwitch::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	return std::make_shared<DigitalSwitch>(
		config["id"].GetString(),
		config["pin"].GetInt());
}

rapidjson::Document DigitalSwitch::to_json() const
{
	rapidjson::Document output = IOutput::to_json();

	console->debug("DigitalSwitch::to_json : serializing output '{}'.", _id.c_str());

	output.AddMember("type", "DigitalSwitch", output.GetAllocator());

	output.AddMember("pin", _pin, output.GetAllocator());

	return output;
}