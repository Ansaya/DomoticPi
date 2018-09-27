#include <MqttAwning.h>

#include <CommFactory.h>
#include <domoticPi.h>
#include <exceptions.h>

using namespace domotic_pi;

const bool MqttAwning::_factoryRegistration =
	OutputFactory::initializer_registration("MqttAwning", MqttAwning::from_json);

MqttAwning::MqttAwning(
	const std::string& id,
	const std::string& mqttTopic,
	std::shared_ptr<MqttComm> mqttComm)
	: IOutput(id), 
	_mqttComm(mqttComm),
	_cmndTopic("cmnd/" + mqttTopic), _statTopic("stat/" + mqttTopic),
	_statSubscription(_mqttComm->subscribe(_statTopic,
		std::bind(&MqttAwning::_stat_message_cb, this, std::placeholders::_1)))
{
	if (mqttComm == nullptr) {
		console->error("MqttAwning::ctor : given mqtt comm interface can not be null.");
		throw domotic_pi_exception("Mqtt comm interface can not be null.");
	}

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_ahkAccessory = std::make_shared<hap::Accessory>();

	_ahkAccessory->addInfoService(getName(), DOMOTIC_PI_APPLE_HOMEKIT_MANUFACTURER,
		typeid(MqttAwning).name(), _id, "1.0.0",
		[this](bool oldValue, bool newValue, void* sender) {
		this->setState(ON);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		this->setState(OFF);
	});

	_ahkAccessory->addWindowCoveringService(&_targetPosition, &_currentPosition, &_positionState, &_nameInfo);

	_nameInfo->setValue(getName());

	_targetPosition->setValueChangeCB([this](int oldValue, int newValue, void* sender) {
		if (oldValue != newValue) {
			setValue(newValue);
		}
	});
#endif // DOMOTIC_PI_APPLE_HOMEKIT

	console->info("MqttAwning::ctor : output '{}' subscribed to topic '{}'.",
		_id.c_str(), _statTopic.c_str());
}

MqttAwning::~MqttAwning()
{
	delete _statSubscription;
}

void MqttAwning::setValue(int newValue)
{
	if (newValue > 100) {
		newValue = 100;
	}
	else if (newValue < 0) {
		newValue = 0;
	}

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_valueLock);
#endif

	std::string message;
	if (newValue > _value) {
		message = "DOWN";
		_direction = 1;
		_targetValue = (uint8_t)newValue;
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		_positionState->setValue(1);
#endif // DOMOTIC_PI_APPLE_HOMEKIT
	}
	else {
		message = "UP";
		_direction = 0;
		_targetValue = (uint8_t)newValue;
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		_positionState->setValue(0);
#endif // DOMOTIC_PI_APPLE_HOMEKIT
	}

	_mqttComm->publish(_cmndTopic, message);

	console->info("MqttAwning::setValue : output '{}' set to '{}'.", _id.c_str(), _value);
}

void MqttAwning::setState(OutState newState)
{
	switch (newState) {
	case ON:
		setValue(100);
		break;
	case OFF:
		setValue(0);
		break;
	case TOGGLE:
		if (_value > 0)
			setValue(0);
		else
			setValue(100);
		break;
	default:
		break;
	}
}

std::shared_ptr<MqttAwning> MqttAwning::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	const rapidjson::Value& mqttInterface = config["comm"];

	std::shared_ptr<MqttComm> mqttComm = nullptr;

	if (mqttInterface.IsString()) {
		mqttComm = std::dynamic_pointer_cast<MqttComm>(parentNode->getComm(mqttInterface.GetString()));
	}
	else {
		mqttComm = std::dynamic_pointer_cast<MqttComm>(CommFactory::from_json(mqttInterface, parentNode));
	}

	return std::make_shared<MqttAwning>(
		config["id"].GetString(),
		config["mqttTopic"].GetString(),
		mqttComm);
}

rapidjson::Document MqttAwning::to_json() const
{
	rapidjson::Document output = IOutput::to_json();

	console->debug("MqttSwitch::to_json : serializing output '{}'.", _id.c_str());

	output.AddMember("type", "MqttAwning", output.GetAllocator());

	rapidjson::Value comm;
	comm.SetString(_mqttComm->getID().c_str(), output.GetAllocator());
	output.AddMember("comm", comm, output.GetAllocator());

	rapidjson::Value mqttTopic;
	mqttTopic.SetString(_cmndTopic.substr(5).c_str(), output.GetAllocator());
	output.AddMember("mqttTopic", mqttTopic, output.GetAllocator());

	return output;
}

void MqttAwning::_stat_message_cb(const struct mosquitto_message * message)
{
	// Read and lowercase received message
	std::string messageString((char *)message->payload, message->payloadlen);

	console->debug("MqttAwning::_stat_message_cb : message '{}' from topic '{}'.",
		messageString.c_str(), _statTopic.c_str());

	// Update local output value with received state
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_valueLock);
#endif

	_value = std::atoi(messageString.c_str());

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_currentPosition->setValue(_value);
#endif

	// Check current and target position and stop moving if target is reached
	if ((_direction == 0 && _value <= _targetValue) || (_direction == 1 && _value >= _targetValue)) {
		
		// Send stop command only if not at limits
		if (_value != 0 && _value != 100) {
			_mqttComm->publish(_cmndTopic, "STOP");
		}
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		_positionState->setValue(2);
#endif // DOMOTIC_PI_APPLE_HOMEKIT

	}

	console->info("MqttAwning::_stat_message_cb : output '{}' changed value to {}.",
		_id.c_str(), _value);
}