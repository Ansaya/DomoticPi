#ifndef DOMOTIC_PI_MQTT_SWITCH
#define DOMOTIC_PI_MQTT_SWITCH

#include "IOutput.h"
#include "MqttComm.h"
#include "MqttSubscription.h"
#include "OutputFactory.h"

#include <mosquitto.h>
#include <string>

namespace domotic_pi {

/**
 *	Output module to access and control a device using MQTT protocol
 *	The device which will be associated with this abstract definition
 *	should have at least a cmnd/name topic where On and OFF commands
 *	will be published. Furthermore if a stat/name topic is implemented
 *	this object will subscribe to it and listen for remote changes; of
 *	course changes on stat/name topic needs to be published as ON/OFF
 *	words to be interpreted correctly.
 */
class MqttSwitch : 
	public IOutput, 
	protected OutputFactory 
{
public:

	/**
	 *	@brief Initialize a new mqtt output switch
	 *
	 *	@param id unique identifier for this module
	 *	@param mqttTopic device topic to be used as suffix after cmnd/ and stat/
	 *	@param mqttComm mqtt comm interface to use for the output
	 */
	MqttSwitch(
		const std::string& id, 
		const std::string& mqttTopic, 
		std::shared_ptr<MqttComm> mqttComm);

	MqttSwitch(const MqttSwitch&) = delete;
	MqttSwitch& operator= (const MqttSwitch&) = delete;
	~MqttSwitch();

	void setState(OutState newState) override;

	void setValue(int newValue) override;

	rapidjson::Document to_json() const override;

private:
	std::shared_ptr<MqttComm> _mqttComm;
	const std::string _cmndTopic;
	const std::string _statTopic;
	MqttSubscription * _statSubscription;
	
	int _range_min;
	int _range_max;

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	hap::BoolCharacteristics_ptr _stateInfo;
#endif

	void _stat_message_cb(const struct mosquitto_message * message);

	static const bool _factoryRegistration;
	static std::shared_ptr<MqttSwitch> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);
};

}

#endif // !DOMOTIC_PI_MQTT_SWITCH
