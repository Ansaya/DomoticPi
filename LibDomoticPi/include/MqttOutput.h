#ifndef DOMOTIC_PI_MQTT_OUTPUT
#define DOMOTIC_PI_MQTT_OUTPUT

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
class MqttOutput : 
	public IOutput, 
	protected OutputFactory 
{
public:

	/**
	 *	@brief Initialize a new output
	 *
	 *	@param id unique identifier for this module
	 *	@param mqttTopic device topic to be used as suffix after cmnd/ and stat/
	 *	@param mqttComm mqtt comm interface to use for the output
	 */
	MqttOutput(
		const std::string& id, 
		const std::string& mqttTopic, 
		std::shared_ptr<MqttComm> mqttComm);

	MqttOutput(const MqttOutput&) = delete;
	MqttOutput& operator= (const MqttOutput&) = delete;
	~MqttOutput();

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
	hap::IntCharacteristics_ptr _valueInfo;
#endif

	void _stat_message_cb(const struct mosquitto_message * message);

	static const bool _factoryRegistration;
	static std::shared_ptr<MqttOutput> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);
};

}

#endif // !DOMOTIC_PI_MQTT_OUTPUT
