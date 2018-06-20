#include <SerialOutput.h>

#include <domoticPi.h>
#include <SerialInterface.h>

using namespace domotic_pi;

SerialOutput::SerialOutput(const std::string& id, SerialInterface_ptr serial, int min_range, int max_range) : 
	Output(id, -1), _serial(serial), _range_min(min_range), _range_max(max_range), _value(min_range)
{
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