#include <MqttSwitch.h>

#include <CommFactory.h>
#include <domoticPi.h>
#include <exceptions.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <thread>

using namespace domotic_pi;

const bool MqttSwitch::_factoryRegistration = 
	OutputFactory::initializer_registration("MqttSwitch", MqttSwitch::from_json);

MqttSwitch::MqttSwitch(
	const std::string& id,
	const std::string& mqttTopic,
	std::shared_ptr<MqttComm> mqttComm) :
	IOutput(id), 
	_mqttComm(mqttComm),
	_cmndTopic("cmnd/" + mqttTopic), _statTopic("stat/" + mqttTopic), 
	_statSubscription(_mqttComm->subscribe(_statTopic,
		std::bind(&MqttSwitch::_stat_message_cb, this, std::placeholders::_1))),
	_range_min(0), _range_max(1)
{
	if (mqttComm == nullptr) {
		console->error("MqttSwitch::ctor : given mqtt comm interface can not be null.");
		throw domotic_pi_exception("Mqtt comm interface can not be null.");
	}

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_ahkAccessory = std::make_shared<hap::Accessory>();

	_ahkAccessory->addInfoService(getName(), DOMOTIC_PI_APPLE_HOMEKIT_MANUFACTURER,
		typeid(MqttSwitch).name(), _id, "1.0.0", 
		[this](bool oldValue, bool newValue, void* sender) {
			this->setState(ON);
			std::this_thread::sleep_for(std::chrono::seconds(2));
			this->setState(OFF);
		});

	_ahkAccessory->addSwitchService(&_stateInfo, &_nameInfo);

	_nameInfo->setValue(getName());

	_stateInfo->setValue(false);
	_stateInfo->setValueChangeCB([this](bool oldValue, bool newValue, void* sender) {
		if (oldValue != newValue)
			setState(newValue ? ON : OFF);
	});
#endif

	console->info("MqttSwitch::ctor : output '{}' subscribed to topic '{}'.", 
		_id.c_str(), _statTopic.c_str());
}

MqttSwitch::~MqttSwitch()
{
	delete _statSubscription;
}

void MqttSwitch::setValue(int newValue)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_valueLock);
#endif

	std::string message;
	if (newValue == _range_min) {
		message = "OFF";
	}
	else {
		message = "ON";
	}

	_mqttComm->publish(_cmndTopic, message);

	console->info("MqttSwitch::setValue : output '{}' set to '{}'.", _id.c_str(), _value);
}

void MqttSwitch::setState(OutState newState)
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

std::shared_ptr<MqttSwitch> MqttSwitch::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	const rapidjson::Value& mqttInterface = config["comm"];

	std::shared_ptr<MqttComm> mqttComm = nullptr;

	if (mqttInterface.IsString()) {
		mqttComm = std::dynamic_pointer_cast<MqttComm>(parentNode->getComm(mqttInterface.GetString()));
	}
	else {
		mqttComm = std::dynamic_pointer_cast<MqttComm>(CommFactory::from_json(mqttInterface, parentNode));
	}

	return std::make_shared<MqttSwitch>(
		config["id"].GetString(),
		config["mqttTopic"].GetString(),
		mqttComm);
}

rapidjson::Document MqttSwitch::to_json() const
{
	rapidjson::Document output = IOutput::to_json();

	console->debug("MqttSwitch::to_json : serializing output '{}'.", _id.c_str());

	output.AddMember("type", "MqttSwitch", output.GetAllocator());

	rapidjson::Value comm;
	comm.SetString(_mqttComm->getID().c_str(), output.GetAllocator());
	output.AddMember("comm", comm, output.GetAllocator());

	rapidjson::Value mqttTopic;
	mqttTopic.SetString(_cmndTopic.substr(5).c_str(), output.GetAllocator());
	output.AddMember("mqttTopic", mqttTopic, output.GetAllocator());

	return output;
}

void MqttSwitch::_stat_message_cb(const struct mosquitto_message * message)
{
	// Read and lowercase received message
	std::string messageString((char *)message->payload, message->payloadlen);

	console->debug("MqttSwitch::_stat_message_cb : message '{}' from topic '{}'.", 
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
	_stateInfo->setValue(_value != _range_min);
#endif

	console->info("MqttSwitch::_stat_message_cb : output '{}' changed value to {}.", 
		_id.c_str(), _value);
}