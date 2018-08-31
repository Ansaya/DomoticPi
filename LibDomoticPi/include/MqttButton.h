#ifndef DOMOTIC_PI_MQTT_INPUT
#define DOMOTIC_PI_MQTT_INPUT

#include "domoticPiDefine.h"
#include "MqttComm.h"
#include "IButtonStateGenerator.h"
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

class MqttButton : 
	public IInput, 
	public IButtonStateGenerator,
	protected InputFactory {
public:
	/**
	 *	@brief Initialize a new mqtt button module
	 *
	 *	@param id module unique id
	 *	@param mqttTopic mqtt topic to listen to (it will be composed as cmnd/mqttTopic)
	 *	@param mqttComm mqtt comm module to use for mqtt connection
	 *	@param doublePressDuration time duration within a double state change is translated to a double press event
	 *	@param longPressDuration time duration current state has to be maintained to trigger a long press event
	 */
	MqttButton(const std::string& id,
		const std::string& mqttTopic,
		std::shared_ptr<MqttComm> mqttComm,
		const std::chrono::milliseconds doublePressDuration = std::chrono::milliseconds::zero(),
		const std::chrono::milliseconds longPressDuration = std::chrono::milliseconds::zero());

	MqttButton(const MqttButton&) = delete;
	MqttButton& operator= (const MqttButton&) = delete;
	virtual ~MqttButton();

	int getValue() const override;

	rapidjson::Document to_json() const override;

private:
	std::shared_ptr<MqttComm> _mqttComm;
	const std::string _cmndTopic;
	MqttSubscription * _cmndSubscription;
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::mutex _isrLock;
#endif
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	hap::IntCharacteristics_ptr _stateInfo;
	CallbackToken_ptr _doublePressToken;
	CallbackToken_ptr _longPressToken;
#endif // DOMOTIC_PI_APPLE_HOMEKIT

	int _value;

	void _stat_message_cb(const struct mosquitto_message * message);

	static const bool _factoryRegistration;
	static std::shared_ptr<MqttButton> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);

};

}

#endif // !DOMOTIC_PI_MQTT_INPUT
