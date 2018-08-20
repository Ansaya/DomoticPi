#include <DigitalOutput.h>

#include <domoticPi.h>

#include <stdexcept>
#include <wiringPi.h>

using namespace domotic_pi;

const bool DigitalOutput::_factoryRegistration =
	OutputFactory::initializer_registration("DigitalOutput", DigitalOutput::from_json);

DigitalOutput::DigitalOutput(const std::string& id, int pinNumber) 
	: Pin(pinNumber), IOutput(id, "DigitalOutput")
{
	if (pinNumber < 0) {
		console->error("DigitalOutput::ctor : pin number must be a valid pin for a digital output.");
		throw std::out_of_range("Pin number must be between 0 and " STR(DOMOTIC_PI_MAX_PIN) ".");
	}

	pinMode(pinNumber, OUTPUT);
	console->info("DigitalOutput::ctor : pin {} set to OUTPUT mode.", pinNumber);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	hap::Service_ptr lightService = std::make_shared<hap::Service>(hap::service_lightBulb);
	_ahkAccessory->addService(lightService);

	lightService->addCharacteristic(_nameInfo);

	_stateInfo = std::make_shared<hap::BoolCharacteristics>(hap::char_on, hap::permission_all);
	_stateInfo->Characteristics::setValue(std::to_string(_value));
	_stateInfo->setValueChangeCB([this](bool oldValue, bool newValue, void* sender) {
		if (oldValue != newValue)
			setState(newValue ? ON : OFF);
	});
	lightService->addCharacteristic(_stateInfo);
#endif
}

DigitalOutput::~DigitalOutput()
{
}

void DigitalOutput::setState(OutState newState)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_valueLock);
#endif

	_value = newState == TOGGLE ? !getValue() : newState;

	digitalWrite(getPin(), _value);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_stateInfo->Characteristics::setValue(std::to_string(_value));
#endif

	console->info("DigitalOutput::setState : output '{}' set to '{}'.", getID(), _value);
}

void DigitalOutput::setValue(int newValue)
{
	if (newValue < 0)
		newValue = 0;

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_valueLock);
#endif

	_value = newValue;

	digitalWrite(getPin(), _value);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_stateInfo->Characteristics::setValue(std::to_string(_value));
#endif

	console->info("DigitalOutput::setValue : output '{}' set to '{}'.", getID(), _value);
}

std::shared_ptr<DigitalOutput> DigitalOutput::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	return std::make_shared<DigitalOutput>(
		config["id"].GetString(),
		config["pin"].GetInt());
}

rapidjson::Document DigitalOutput::to_json() const
{
	rapidjson::Document output = IOutput::to_json();

	console->debug("DigitalOutput::to_json : serializing output '{}'.", _id.c_str());

	output.AddMember("type", "DigitalOutput", output.GetAllocator());

	output.AddMember("pin", _pin, output.GetAllocator());

	return output;
}