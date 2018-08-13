#ifndef DOMOTIC_PI_MQTT_OUTPUT
#define DOMOTIC_PI_MQTT_OUTPUT

#include "IMqtt.h"
#include "MqttSubscription.h"
#include "Output.h"

#include <mosquitto.h>
#include <string>

namespace domotic_pi {

class MqttOutput : public Output, public IMqtt {
public:
	MqttOutput(
		const std::string& id, 
		const std::string& mqttTopic, 
		const std::string& mqttBroker, 
		const int mqttPort, 
		const std::string& mqttUsername = "", 
		const std::string& mqttPassword = "");

	~MqttOutput();

	MqttOutput(const MqttOutput&) = delete;
	MqttOutput& operator= (const MqttOutput&) = delete;

	void setState(OutState newState) override;

	void setValue(int newValue) override;

	rapidjson::Document to_json() const override;

private:
	MqttSubscription * _statSubscription;
	const std::string _cmndTopic;
	const std::string _statTopic;
	int _range_min;
	int _range_max;

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	hap::BoolCharacteristics_ptr _stateInfo;
	hap::IntCharacteristics_ptr _valueInfo;
#endif

	void _stat_message_cb(const struct mosquitto_message * message);

};

}

#endif // !DOMOTIC_PI_MQTT_OUTPUT
