#include "DigitalOutput.h"

#include "domoticPi.h"

#include <stdexcept>
#include <wiringPi.h>

using namespace domotic_pi;

DigitalOutput::DigitalOutput(const std::string& id, int pinNumber) : Output(id, pinNumber)
{
	if (pinNumber < 0) {
		console->error("DigitalOutput::ctor : pin number must be a valid pin for a digital output.");
		throw std::out_of_range("Pin number must be between 0 and " STR(DOMOTIC_PI_MAX_PIN) ".");
	}

	pinMode(pinNumber, OUTPUT);
	console->info("DigitalOutput::ctor : pin %d set to OUTPUT mode.", pinNumber);
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

	console->info("DigitalOutput::setState : output '%s' set to '%d'.", getID(), _value);
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

	console->info("DigitalOutput::setValue : output '%s' set to '%d'.", getID(), _value);
}

rapidjson::Document DigitalOutput::to_json() const
{
	rapidjson::Document output = Output::to_json();

	console->debug("DigitalOutput::to_json : serializing output '%s'.", _id.c_str());

	output.AddMember("type", "digital", output.GetAllocator());

	output.AddMember("pin", _pin, output.GetAllocator());

	return output;
}