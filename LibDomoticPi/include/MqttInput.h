#ifndef DOMOTIC_PI_MQTT_INPUT
#define DOMOTIC_PI_MQTT_INPUT

#include "IMqtt.h"
#include "Input.h"
#include "MqttSubscription.h"

#include <functional>
#include <list>
#include <mosquitto.h>
#include <mutex>
#include <string>

namespace domotic_pi {

class MqttInput : public Input, public IMqtt {
public:
	MqttInput(const std::string& id,
		const std::string& mqttTopic,
		const std::string& mqttBroker,
		const int mqttPort,
		const std::string& mqttUsername = "",
		const std::string& mqttPassword = "");

	MqttInput(const MqttInput&) = delete;
	MqttInput& operator= (const MqttInput&) = delete;
	~MqttInput();

	int getValue() const;

	void addISRCall(std::function<void()> cb, int isr_mode);

	void setISRMode(int isr_mode);

private:
	MqttSubscription * _cmndSubscription;
	const std::string _cmndTopic;
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::mutex _isrLock;
#endif
	int _value;
	int _isrMode;
	std::list<std::function<void()>> _isrCB;

	void _stat_message_cb(const struct mosquitto_message * message);

};

}

#endif // !DOMOTIC_PI_MQTT_INPUT
