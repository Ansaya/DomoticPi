#include <MqttOutput.h>

#include <CommFactory.h>
#include <domoticPi.h>
#include <exceptions.h>

#include <algorithm>
#include <functional>

using namespace domotic_pi;

const bool MqttOutput::_factoryRegistration = 
	OutputFactory::initializer_registration("MqttOutput", MqttOutput::from_json);

MqttOutput::MqttOutput(
	const std::string& id,
	const std::string& mqttTopic,
	std::shared_ptr<MqttComm> mqttComm) :
	IOutput(id, "MqttOutput"), 
	_mqttComm(mqttComm),
	_cmndTopic("cmnd/" + mqttTopic), _statTopic("stat/" + mqttTopic), 
	_statSubscription(_mqttComm->subscribe(_statTopic,
		std::bind(&MqttOutput::_stat_message_cb, this, std::placeholders::_1))),
	_range_min(0), _range_max(1)
{
	if (mqttComm == nullptr) {
		console->error("MqttOutput::ctor : given mqtt comm interface can not be null.");
		throw domotic_pi_exception("Mqtt comm interface can not be null.");
	}

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

	_mqttComm->publish(_cmndTopic, message);

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

std::shared_ptr<MqttOutput> MqttOutput::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	const rapidjson::Value& mqttInterface = config["comm"];

	std::shared_ptr<MqttComm> mqttComm = nullptr;

	if (mqttInterface.IsString()) {
		mqttComm = std::dynamic_pointer_cast<MqttComm>(parentNode->getComm(mqttInterface.GetString()));
	}
	else {
		mqttComm = std::dynamic_pointer_cast<MqttComm>(CommFactory::from_json(mqttInterface, parentNode));
	}

	return std::make_shared<MqttOutput>(
		config["id"].GetString(),
		config["mqttTopic"].GetString(),
		mqttComm);
}

rapidjson::Document MqttOutput::to_json() const
{
	rapidjson::Document output = IOutput::to_json();

	console->debug("MqttOutput::to_json : serializing output '{}'.", _id.c_str());

	rapidjson::Value comm;
	comm.SetString(_mqttComm->getID().c_str(), output.GetAllocator());
	output.AddMember("comm", comm, output.GetAllocator());

	rapidjson::Value mqttTopic;
	mqttTopic.SetString(_cmndTopic.substr(5).c_str(), output.GetAllocator());
	output.AddMember("mqttTopic", mqttTopic, output.GetAllocator());

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