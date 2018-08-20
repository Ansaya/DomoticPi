#ifndef DOMOTIC_PI_MQTT_VOLUME
#define DOMOTIC_PI_MQTT_VOLUME

#include "IMqtt.h"
#include "MqttSubscription.h"
#include "IOutput.h"

#include <memory>
#include <mosquitto.h>

namespace domotic_pi {

class MqttVolume : public IOutput, public IMqtt {
public:
	MqttVolume(
		const std::string& id,
		const std::string& mqttVolumeTopic,
		const std::string& mqttMuteeTopic,
		const std::string& mqttPowerTopic,
		const std::string& mqttBroker,
		const int mqttPort,
		const std::string& mqttUsername = "",
		const std::string& mqttPassword = "");

	MqttVolume(const MqttVolume&) = delete;
	MqttVolume& operator= (const MqttVolume&) = delete;
	~MqttVolume();

	void setState(OutState newState) override;

	void setValue(int newValue) override;

	rapidjson::Document to_json() const override;

private:
	MqttSubscription * _volumeStat;
	const std::string _volCmndTopic;
	const std::string _volStatTopic;

	MqttSubscription * _muteStat;
	const std::string _muteCmndTopic;
	const std::string _muteStatTopic;

	MqttSubscription * _powerStat;
	const std::string _powerCmndTopic;
	const std::string _powerStatTopic;
	
	int _range_min;
	int _range_max;

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	hap::BoolCharacteristics_ptr _stateInfo;
	hap::IntCharacteristics_ptr _valueInfo;
#endif

	void _vol_stat_cb(const struct mosquitto_message * message);
	void _mute_stat_cb(const struct mosquitto_message * message);
	void _power_stat_cb(const struct mosquitto_message * message);

	static const bool _factoryRegistration;
	static std::shared_ptr<MqttVolume> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);

};

}

#endif // !DOMOTIC_PI_MQTT_VOLUME
