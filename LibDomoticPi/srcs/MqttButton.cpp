#include <MqttButton.h>

#include <CommFactory.h>
#include <domoticPi.h>

#include <exception>
#include <exceptions.h>
#include <tuple>
#include <wiringPi.h>

using namespace domotic_pi;

const bool MqttButton::_factoryRegistration = 
	InputFactory::initializer_registration("MqttButton", MqttButton::from_json);

MqttButton::MqttButton(const std::string& id,
	const std::string& mqttTopic,
	std::shared_ptr<MqttComm> mqttComm,
	const std::chrono::milliseconds doublePressDuration,
	const std::chrono::milliseconds longPressDuration) :
	IInput(id), 
	IButtonStateGenerator(doublePressDuration, longPressDuration),
	_mqttComm(mqttComm), 
	_cmndTopic("cmnd/" + mqttTopic), 
	_cmndSubscription(_mqttComm->subscribe(_cmndTopic,
		std::bind(&MqttButton::_stat_message_cb, this, std::placeholders::_1))),
	_value(0)
{
	if (mqttComm == nullptr) {
		console->error("MqttButton::ctor : given mqtt comm interface can not be null.");
		throw domotic_pi_exception("Mqtt comm interface can not be null.");
	}

	console->info("MqttButton::ctor : output '{}' subscribed to topic '{}'.",
		getID().c_str(), _cmndTopic.c_str());

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_ahkAccessory = std::make_shared<hap::Accessory>();

	_ahkAccessory->addInfoService(getName(), DOMOTIC_PI_APPLE_HOMEKIT_MANUFACTURER,
		typeid(MqttButton).name(), _id, "1.0.0", [](bool oldValue, bool newValue, void* sender) {});

	_ahkAccessory->addStatelessSwitchService(&_stateInfo, &_nameInfo);

	_nameInfo->setValue(getName());

	_doublePressToken = addDoublePressCallback([this] {
		_stateInfo->setValue(double_press);
	});

	_longPressToken = addLongPressCallback([this] {
		_stateInfo->setValue(long_press);
	});
#endif
}

MqttButton::~MqttButton()
{
	delete _cmndSubscription;
}

int MqttButton::getValue() const
{
	return _value;
}

std::shared_ptr<MqttButton> MqttButton::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	const rapidjson::Value& mqttInterface = config["comm"];

	std::shared_ptr<MqttComm> mqttComm = nullptr;

	if (mqttInterface.IsString()) {
		mqttComm = std::dynamic_pointer_cast<MqttComm>(parentNode->getComm(mqttInterface.GetString()));
	}
	else {
		mqttComm = std::dynamic_pointer_cast<MqttComm>(CommFactory::from_json(mqttInterface, parentNode));
	}

	auto durations = IButtonStateGenerator::from_json(config);

	return std::make_shared<MqttButton>(
		config["id"].GetString(),
		config["mqttTopic"].GetString(),
		mqttComm,
		std::get<0>(durations),
		std::get<1>(durations));
}

rapidjson::Document MqttButton::to_json() const
{
	rapidjson::Document input = IInput::to_json();

	// Add double/long press duration from superclass
	rapidjson::Document stateDurations = IButtonStateGenerator::to_json();
	for (auto attr = stateDurations.MemberBegin(); attr != stateDurations.MemberEnd(); ++attr) {
		input.AddMember(attr->name, attr->value.Move(), input.GetAllocator());
	}

	console->debug("MqttButton::to_json : serializing input '{}'.", _id.c_str());

	input.AddMember("type", "MqttButton", input.GetAllocator());

	rapidjson::Value comm;
	comm.SetString(_mqttComm->getID().c_str(), input.GetAllocator());
	input.AddMember("comm", comm, input.GetAllocator());

	rapidjson::Value mqttTopic;
	mqttTopic.SetString(_cmndTopic.substr(5).c_str(), input.GetAllocator());
	input.AddMember("mqttTopic", mqttTopic, input.GetAllocator());

	return input;
}

void MqttButton::_stat_message_cb(const struct mosquitto_message * message)
{
	// Read and lowercase received message
	std::string messageString((char *)message->payload, message->payloadlen);

	console->debug("MqttButton::_stat_message_cb : message '{}' from topic '{}'.",
		messageString.c_str(), _cmndTopic.c_str());

	std::transform(messageString.begin(), messageString.end(), messageString.begin(), ::tolower);

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_isrLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	_value = !_value;

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_stateInfo->setValue(0);
#endif // DOMOTIC_PI_APPLE_HOMEKIT

	console->info("MqttButton::_stat_message_cb : input '{}' changed value to '{}'.", getID().c_str(), _value);

	valueChangeCallbacks(_value);
}