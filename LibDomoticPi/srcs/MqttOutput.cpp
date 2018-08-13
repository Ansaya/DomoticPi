#include <MqttOutput.h>

#include <algorithm>
#include <functional>

using namespace domotic_pi;

MqttOutput::MqttOutput(
	const std::string& id,
	const std::string& mqttTopic,
	const std::string& mqttBroker,
	const int mqttPort,
	const std::string& mqttUsername,
	const std::string& mqttPassword) :
	Output(id, -1), 
	IMqtt(id, mqttBroker, mqttPort, mqttUsername, mqttPassword),
	_cmndTopic("cmnd/" + mqttTopic), _statTopic("stat/" + mqttTopic), _range_min(0), _range_max(1)
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

	console->info("MqttOutput::ctor : output '{}' subscribed to topic '{}'.", 
		getID().c_str(), _statTopic.c_str());
	
	_statSubscription = subscribe(_statTopic, 
		std::bind(&MqttOutput::_stat_message_cb, this, std::placeholders::_1));
}

MqttOutput::~MqttOutput()
{
	delete _statSubscription;
}

void MqttOutput::setValue(int newValue)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_valueLock);
#endif

	std::string message;
	if (newValue == _range_min) {
		message = "OFF";
		_value = 0;
	}
	else {
		message = "ON";
		_value = 1;
	}

	publish(_cmndTopic, message);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_stateInfo->hap::Characteristics::setValue(std::to_string(_value != _range_min));
	_valueInfo->hap::Characteristics::setValue(std::to_string(_value));
#endif

	console->info("MqttOutput::setValue : output '{}' set to '{}'.", getID(), _value);
}

void MqttOutput::setState(OutState newState)
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

rapidjson::Document MqttOutput::to_json() const
{
	rapidjson::Document output = Output::to_json();

	console->debug("MqttOutput::to_json : serializing output '{}'.", _id.c_str());

	output.AddMember("type", "mqtt", output.GetAllocator());

	rapidjson::Document mqttInterface;
	rapidjson::Value mqttBroker;
	rapidjson::Value mqttPort;
	rapidjson::Value mqttTopic;
	mqttBroker.SetString(getHost().c_str(), output.GetAllocator());
	mqttPort.SetInt(getPort());
	mqttTopic.SetString(_cmndTopic.substr(5).c_str(), output.GetAllocator());
	mqttInterface.AddMember("broker", mqttBroker, output.GetAllocator());
	mqttInterface.AddMember("port", mqttPort, output.GetAllocator());
	mqttInterface.AddMember("topic", mqttTopic, output.GetAllocator());

	output.AddMember("mqttInterface", mqttInterface, output.GetAllocator());

	return output;
}

void MqttOutput::_stat_message_cb(const struct mosquitto_message * message)
{
	// Read and lowercase received message
	std::string messageString((char *)message->payload, message->payloadlen);

	console->debug("MqttOutput::_stat_message_cb : message '{}' from topic '{}'.", 
		messageString.c_str(), _statTopic.c_str());

	std::transform(messageString.begin(), messageString.end(), messageString.begin(), ::tolower);

	// Update local output value with received state
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_valueLock);
#endif

	if (messageString.compare("on") == 0) {
		_value = _range_max;
	}
	else if (messageString.compare("off") == 0) {
		_value = _range_min;
	}
	else {
		_value = std::atoi(messageString.c_str());
	}

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_stateInfo->hap::Characteristics::setValue(std::to_string(_value != _range_min));
	_valueInfo->hap::Characteristics::setValue(std::to_string(_value));
#endif

	console->info("MqttOutput::_stat_message_cb : output '{}' changed value to {}.", 
		getID().c_str(), _value);
}