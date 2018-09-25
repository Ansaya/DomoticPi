#ifndef DOMOTIC_PI_MQTT_AWNING
#define DOMOTIC_PI_MQTT_AWNING

#include "IOutput.h"
#include "MqttComm.h"
#include "MqttSubscription.h"
#include "OutputFactory.h"

#include <mosquitto.h>
#include <string>

namespace domotic_pi {

	class MqttAwning : public IOutput, protected OutputFactory {
	public:

		/**
		 *	@brief Initialize a new mqtt awning
		 *
		 *	@param id unique identifier for this module
		 *	@param mqttTopic device topic to be used as suffix after cmnd/ and stat/
		 *	@param mqttComm mqtt comm interface to use for the output
		 */
		MqttAwning(
			const std::string& id,
			const std::string& mqttTopic,
			std::shared_ptr<MqttComm> mqttComm);

		MqttAwning(const MqttAwning&) = delete;
		MqttAwning& operator= (const MqttAwning&) = delete;
		~MqttAwning();

		void setState(OutState newState) override;

		void setValue(int newValue) override;

		rapidjson::Document to_json() const override;

	private:
		std::shared_ptr<MqttComm> _mqttComm;
		const std::string _cmndTopic;
		const std::string _statTopic;
		uint8_t _direction;
		uint8_t _targetValue;
		MqttSubscription * _statSubscription;

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		hap::IntCharacteristics_ptr _currentPosition;
		hap::IntCharacteristics_ptr _targetPosition;
		hap::IntCharacteristics_ptr _positionState;
#endif

		void _stat_message_cb(const struct mosquitto_message * message);

		static const bool _factoryRegistration;
		static std::shared_ptr<MqttAwning> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);
	};

}

#endif // !DOMOTIC_PI_MQTT_AWNING
