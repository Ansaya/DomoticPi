#ifndef DOMOTIC_PI_MQTT_MODULE
#define DOMOTIC_PI_MQTT_MODULE

#include "exceptions.h"
#include "Module.h"
#include "MqttSubscription.h"

#include <mosquitto.h>

namespace domotic_pi {

template<class T>
class MqttModule : public Module<T> {
public:
	MqttModule(
		const std::string& id, 
		const std::string& host, 
		const int port, 
		const std::string& username = "",
		const std::string& password = "");
	virtual ~MqttModule();

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
	 *	@note The returned object must not be shared outside the class: MqttSubscription object
	 *		  need to be disposed along with its relative MqttModule class to avoid runtime errors.
	 *
	 *	@return Subscription object to be kept until the subscription is needed
	 */
	MqttSubscription subscribe(const std::string& topic, 
					std::function<void(const struct mosquitto_message *)> message_cb);

private:
	const std::string _host;
	const int _port;
	const std::string username;
	const std::string password;
	struct mosquitto *mosq;

};

}

template<class T>
domotic_pi::MqttModule<T>::MqttModule(
	const std::string& id,
	const std::string& host,
	const int port,
	const std::string& username = "",
	const std::string& password = "") :
	domotic_pi::Module(id), _host(host), _port(port)
{
	// Allocate mosquitto structure for the new connection
	mosq = mosquitto_new(_id.c_str(), true, NULL);
	if (!mosq) {
		domotic_pi::console->error("MqttModule::ctor : module '%s' could not allocate memory for mqtt structure.", 
			_id.c_str());

		throw domotic_pi_exception("Connection to mqtt broker failed.");
	}

	// Set username and password if required
	if (!username.empty()) {
		mosquitto_username_pw_set(mosq, username.c_str(), password.c_str());
	}

	// Connect to the required mqtt broker
	int res = mosquitto_connect(mosq, host.c_str(), port, 60);
	if (ress != MOSQ_ERR_SUCCESS) {
		domotic_pi::console->error("MqttModule::ctor : module '%s' could not connect to borker at %s:%d : %s",
			_id.c_str(), host.c_str(), port, mosquitto_strerror(res));

		throw domotic_pi_exception("Connection to mqtt broker failed.");
	}

	// Start mqtt listener thread for the new connection
	res = mosquitto_loop_start(mosq);
	if (res != MOSQ_ERR_SUCCESS) {
		domotic_pi::console->error("MqttModule::ctor : could not start mqtt client thread for module '%s' : %s",
			_id.c_str(), mosquitto_strerror(res));
		throw domotic_pi_exception("Thread start failed for mqtt listener.");
	}
}

template<class T>
domotic_pi::MqttModule<T>::~MqttModule()
{
	// Disconnect mqtt client and dispose listener thread
	bool forceStop = mosquitto_disconnect(mosq) != MOSQ_ERR_SUCCESS;
	mosquitto_loop_stop(mosq, forceStop);

	// Deallocate mosquitto structure for the closed connection
	mosquitto_destroy(mosq);
}

template<class T>
domotic_pi::MqttModule<T>::publish(
	const std::string& topic, 
	const std::string& message, 
	bool retain)
{
	mosquitto_publish(mosq, 
		NULL,						// For now no message id is required
		topic.c_str(),				// Set required topic to publish to
		message.length(),			
		(void *)message.c_str(),		
		2,							// Set QoS to deliver the message exactly once
		retain);

	domotic_pi::console->info("MqttModule::publish : message '%s' published on topic '%s'.", 
		message.c_str(), topic.c_str());
}

template<class T>
domotic_pi::MqttSubscription domotic_pi::MqttModule<T>::subscribe(
	const std::string& topic,
	std::function<void(const struct mosquitto_message *)> message_cb)
{
	if (!username.empty()) {
		return domotic_pi::MqttSubscription(_host, _port, topic, message_cb, username, password);
	}

	return domotic_pi::MqttSubscription(_host, _port, topic, message_cb);
}


#endif // !DOMOTIC_PI_MQTT_MODULE
