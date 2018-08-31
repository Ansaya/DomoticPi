#ifndef DOMOTIC_PI_MQTT_VOLUME
#define DOMOTIC_PI_MQTT_VOLUME

#include "domoticPiDefine.h"
#include "MqttComm.h"
#include "IOutput.h"
#include "MqttSubscription.h"
#include "OutputFactory.h"

#include <memory>
#include <mosquitto.h>

namespace domotic_pi {

class MqttVolume : public IOutput, protected OutputFactory {
public:
	MqttVolume(
		const std::string& id,
		const std::string& mqttVolumeTopic,
		std::shared_ptr<MqttComm> mqttComm);

	MqttVolume(const MqttVolume&) = delete;
	MqttVolume& operator= (const MqttVolume&) = delete;
	~MqttVolume();

	void setState(OutState newState) override;

	void setValue(int newValue) override;

	rapidjson::Document to_json() const override;

private:
	std::shared_ptr<MqttComm> _mqttComm;
	const std::string _volCmndTopic;
	const std::string _volStatTopic;
	MqttSubscription * _volumeStat;
	
	int _range_min;
	int _range_max;

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	hap::BoolCharacteristics_ptr _stateInfo;
#endif

	void _vol_stat_cb(const struct mosquitto_message * message);

	static const bool _factoryRegistration;
	static std::shared_ptr<MqttVolume> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);

};

}

#endif // !DOMOTIC_PI_MQTT_VOLUME
