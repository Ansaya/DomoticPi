#ifndef DOMOTIC_PI_MQTT_MODULE
#define DOMOTIC_PI_MQTT_MODULE

#include "CommFactory.h"
#include "IComm.h"
#include "MqttLib.h"
#include "MqttSubscription.h"
#include "Serializable.h"

#include <functional>
#include <memory>
#include <mosquitto.h>
#include <rapidjson/document.h>
#include <string>

namespace domotic_pi {

class MqttComm : public IComm, protected CommFactory {
public:
	MqttComm(
		const std::string& id,
		const std::string& host, 
		const int port, 
		const std::string& username = "",
		const std::string& password = "");

	MqttComm(const MqttComm&) = delete;
	MqttComm& operator= (const MqttComm&) = delete;
	virtual ~MqttComm();

	const std::string &getHost() const;

	int getPort() const;

	const std::string &getUsername() const;

	const std::string &getPassword() const;

	/**
	 *	@brief Publish given message to specified topic (with retain flag)
	 *
	 *	@param topic topic to publish the message to
	 *	@param message message to be published
	 *	@param retain retian flag value
	 *
	 */
	void publish(const std::string& topic, const std::string& message, bool retain = true);

	/**
	 *	@brief Subscribe to a topic and set a callback for received messages
	 *
	 *	@param topic topic to subscribe to
	 *	@param message_cb callback to trigger on received messages
	 *
	 *	@note The returned object must not be shared outside the class: MqttComm object
	 *		  need to be disposed along with its relative MqttModule class to avoid runtime errors.
	 *
	 *	@return Subscription object to be kept until the subscription is needed
	 */
	MqttSubscription * subscribe(const std::string& topic, 
					std::function<void(const struct mosquitto_message *)> message_cb);

	rapidjson::Document to_json() const override;

private:
	const std::shared_ptr<MqttLib> _mosquittoLib;
	const std::string _host;
	const int _port;
	const std::string _username;
	const std::string _password;
	struct mosquitto *mosq;

	static const bool _factoryRegistration;
	static std::shared_ptr<MqttComm> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);

};

}


#endif // !DOMOTIC_PI_MQTT_MODULE
