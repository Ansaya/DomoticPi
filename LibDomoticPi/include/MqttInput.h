#ifndef DOMOTIC_PI_MQTT_INPUT
#define DOMOTIC_PI_MQTT_INPUT

#include "domoticPiDefine.h"
#include "MqttComm.h"
#include "IInput.h"
#include "InputFactory.h"
#include "MqttSubscription.h"

#include <functional>
#include <list>
#include <memory>
#include <mosquitto.h>
#ifdef DOMOTIC_PI_THREAD_SAFE
#include <mutex>
#endif // DOMOTIC_PI_THREAD_SAFE
#include <rapidjson/document.h>
#include <string>

namespace domotic_pi {

class MqttInput : public IInput, protected InputFactory {
public:
	MqttInput(const std::string& id,
		const std::string& mqttTopic,
		std::shared_ptr<MqttComm> mqttComm);

	MqttInput(const MqttInput&) = delete;
	MqttInput& operator= (const MqttInput&) = delete;
	virtual ~MqttInput();

	int getValue() const override;

	void addISRCall(std::function<void()> cb, int isr_mode) override;

	void setISRMode(int isr_mode) override;

	rapidjson::Document to_json() const override;

private:
	std::shared_ptr<MqttComm> _mqttComm;
	const std::string _cmndTopic;
	MqttSubscription * _cmndSubscription;
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
