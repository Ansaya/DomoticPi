#include <MqttInput.h>

#include <domoticPi.h>

#include <exception>
#include <wiringPi.h>

using namespace domotic_pi;

MqttInput::MqttInput(const std::string& id,
	const std::string& mqttTopic,
	const std::string& mqttBroker,
	const int mqttPort,
	const std::string& mqttUsername,
	const std::string& mqttPassword) : 
	IInput(id, "MqttInput"), IMqtt(id, mqttBroker, mqttPort, mqttUsername, mqttPassword), 
	_cmndTopic("cmnd/" + mqttTopic), _value(0)
{
	console->info("MqttInput::ctor : output '{}' subscribed to topic '{}'.",
		getID().c_str(), _cmndTopic.c_str());

	_cmndSubscription = subscribe(_cmndTopic,
		std::bind(&MqttInput::_stat_message_cb, this, std::placeholders::_1));
}

MqttInput::~MqttInput()
{
	delete _cmndSubscription;
}

int MqttInput::getValue() const
{
	return _value;
}

void MqttInput::addISRCall(std::function<void()> cb, int isr_mode)
{
	if (cb == nullptr) {
		console->error("MqttInput::addISRCall : setter called with null function pointer. "
			"(Input: '{}')", getID().c_str());
		throw std::invalid_argument("Callback function must not be null.");
	}

	if (isr_mode < 0 || isr_mode > 4) {
		console->error("MqttInput::addISRCall : setter called with invalid isr mode '{}'. "
			"(Input: '{}')", getID().c_str());
		throw std::invalid_argument("isr_mode must be a value from 0 to 4");
	}

	// If requested isr mode is none, disable isr and complete the call
	if (isr_mode == INT_EDGE_NONE)
		return setISRMode(isr_mode);

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_isrLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	_isrMode = isr_mode;
	_isrCB.push_back(cb);

	console->info("MqttInput::addISRCall : ISR action added to input '{}'.", getID().c_str());
}

void MqttInput::setISRMode(int isr_mode)
{
	console->info("MqttInput::setISRMode : isr mode '{}' requested.", isr_mode);

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_isrLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	_isrMode = isr_mode;

	if (_isrMode == INT_EDGE_NONE) {
		_isrCB.clear();
	}
}

rapidjson::Document MqttInput::to_json() const
{
	rapidjson::Document input = IInput::to_json();

	console->debug("MqttInput::to_json : serializing input '{}'.", _id.c_str());

	input.AddMember("type", "MqttInput", input.GetAllocator());

	rapidjson::Document mqttInterface;
	rapidjson::Value mqttBroker;
	rapidjson::Value mqttPort;
	rapidjson::Value mqttTopic;
	mqttBroker.SetString(getHost().c_str(), input.GetAllocator());
	mqttPort.SetInt(getPort());
	mqttTopic.SetString(_cmndTopic.substr(5).c_str(), input.GetAllocator());
	mqttInterface.AddMember("broker", mqttBroker, input.GetAllocator());
	mqttInterface.AddMember("port", mqttPort, input.GetAllocator());
	mqttInterface.AddMember("topic", mqttTopic, input.GetAllocator());

	input.AddMember("mqttInterface", mqttInterface, input.GetAllocator());

	return input;
}

void MqttInput::_stat_message_cb(const struct mosquitto_message * message)
{
	// Read and lowercase received message
	std::string messageString((char *)message->payload, message->payloadlen);

	console->debug("MqttInput::_stat_message_cb : message '{}' from topic '{}'.",
		messageString.c_str(), _cmndTopic.c_str());

	std::transform(messageString.begin(), messageString.end(), messageString.begin(), ::tolower);

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_isrLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	_value = !_value;

	console->info("MqttInput::_stat_message_cb : input '{}' changed value to '{}'.", getID().c_str(), _value);

	bool _isrTrigger = _isrMode == INT_EDGE_BOTH ||
		(_isrMode == INT_EDGE_FALLING && _value == 0) ||
		(_isrMode == INT_EDGE_RISING && _value == 1);

	if (_isrTrigger) {
		console->info("MqttInput::_stat_message_cb : ISR call execution for input '{}'.", getID().c_str());
		for (auto cb : _isrCB) {
			try {
				cb();
			}
			catch (std::exception& e) {
				console->warn("MqttInput::input_ISR : isr call throw the following exception : {}", e.what());
			}
		}
	}
}

std::shared_ptr<MqttInput> MqttInput::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	const rapidjson::Value& mqttInterface = config["mqttInterface"];

	if (mqttInterface.HasMember("username")) {
		return std::make_shared<MqttInput>(
			config["id"].GetString(),
			mqttInterface["topic"].GetString(),
			mqttInterface["broker"].GetString(),
			mqttInterface["port"].GetInt(),
			mqttInterface["username"].GetString(),
			mqttInterface["password"].GetString());
	}
	else {
		return std::make_shared<MqttInput>(
			config["id"].GetString(),
			mqttInterface["topic"].GetString(),
			mqttInterface["broker"].GetString(),
			mqttInterface["port"].GetInt());
	}
}