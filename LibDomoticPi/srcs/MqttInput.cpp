#include <MqttInput.h>

#include <exception>
#include <wiringPi.h>

using namespace domotic_pi;

MqttInput::MqttInput(const std::string& id,
	const std::string& mqttTopic,
	const std::string& mqttBroker,
	const int mqttPort,
	const std::string& mqttUsername,
	const std::string& mqttPassword) : 
	Input(id, -1), IMqtt(id, mqttBroker, mqttPort, mqttUsername, mqttPassword), 
	_value(0), _cmndTopic("cmnd/" + mqttTopic)
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
	console->info("DigitalInput::setISRMode : isr mode '{}' requested.", isr_mode);

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_isrLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	_isrMode = isr_mode;

	if (_isrMode == INT_EDGE_NONE) {
		_isrCB.clear();
	}
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