#ifndef DOMOTIC_PI_MQTT_INPUT
#define DOMOTIC_PI_MQTT_INPUT

#include "domoticPiDefine.h"
#include "IMqtt.h"
#include "IInput.h"
#include "InputFactory.h"
#include "MqttSubscription.h"

#include <functional>
#include <list>
#include <mosquitto.h>
#ifdef DOMOTIC_PI_THREAD_SAFE
#include <mutex>
#endif // DOMOTIC_PI_THREAD_SAFE
#include <rapidjson/document.h>
#include <string>

namespace domotic_pi {

class MqttInput : public IInput, public IMqtt, protected InputFactory {
public:
	MqttInput(const std::string& id,
		const std::string& mqttTopic,
		const std::string& mqttBroker,
		const int mqttPort,
		const std::string& mqttUsername = "",
		const std::string& mqttPassword = "");

	MqttInput(const MqttInput&) = delete;
	MqttInput& operator= (const MqttInput&) = delete;
	virtual ~MqttInput();

	int getValue() const override;

	void addISRCall(std::function<void()> cb, int isr_mode) override;

	void setISRMode(int isr_mode) override;

	rapidjson::Document to_json() const override;

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

	static const bool _factoryRegistration;
	static std::shared_ptr<MqttInput> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);

};

}

#endif // !DOMOTIC_PI_MQTT_INPUT
