#include "SerialOutput.h"

#include "domoticPi.h"
#include "SerialInterface.h"

using namespace domotic_pi;

SerialOutput::SerialOutput(const std::string& id, SerialInterface_ptr serial, int min_range, int max_range) : 
	Output(id, -1), _serial(serial), _range_min(min_range), _range_max(max_range), _value(min_range)
{
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

	console->info("SerialOutput::setValue : output '%s' set to '%d'.", getID(), _value);
}

rapidjson::Document SerialOutput::to_json() const
{
	rapidjson::Document output = Output::to_json();

	console->debug("SerialOutput::to_json : serializing output '%s'.", _id.c_str());

	output.AddMember("type", "serial", output.GetAllocator());

	rapidjson::Value serialInterface;
	serialInterface.SetString(_serial->getPort().c_str(), output.GetAllocator());
	output.AddMember("serialInterface", serialInterface, output.GetAllocator());

	output.AddMember("range_min", _range_min, output.GetAllocator());
	output.AddMember("range_max", _range_max, output.GetAllocator());

	return output;
}