#ifndef DOMOTIC_PI_MQTT_MODULE
#define DOMOTIC_PI_MQTT_MODULE

#include "MqttLib.h"
#include "MqttSubscription.h"

#include <functional>
#include <memory>
#include <mosquitto.h>
#include <string>

namespace domotic_pi {

class IMqtt {
public:
	IMqtt(
		const std::string& id, 
		const std::string& host, 
		const int port, 
		const std::string& username = "",
		const std::string& password = "");

	IMqtt(const IMqtt&) = delete;
	IMqtt& operator= (const IMqtt&) = delete;
	virtual ~IMqtt();

	const std::string &getHost() const;

	int getPort() const;

	const std::string &getUsername() const;

	const std::string &getPassword() const;

protected:

	/**
	 *	@brief Publish given message to specified topic (with retain flag if needed)
	 *
	 *	@param topic topic to publish the message to
	 *	@param message message to be published
	 *	@param retain retian flag
	 *
	 */
	void publish(const std::string& topic, const std::string& message, bool retain = false);

	/**
	 *	@brief Subscribe to a topic and set a callback for received messages
	 *
	 *	@param topic topic to subscribe to
	 *	@param message_cb callback to trigger on received messages
	 *
	 *	@note The returned object must not be shared outside the class: IMqtt object
	 *		  need to be disposed along with its relative MqttModule class to avoid runtime errors.
	 *
	 *	@return Subscription object to be kept until the subscription is needed
	 */
	MqttSubscription *subscribe(const std::string& topic, 
					std::function<void(const struct mosquitto_message *)> message_cb);

private:
	const std::shared_ptr<MqttLib> _mosquittoLib;
	const std::string _host;
	const int _port;
	const std::string _username;
	const std::string _password;
	struct mosquitto *mosq;

};

}


#endif // !DOMOTIC_PI_MQTT_MODULE
