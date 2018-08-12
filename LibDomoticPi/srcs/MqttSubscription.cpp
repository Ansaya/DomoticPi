#include <MqttSubscription.h>

#include <domoticPi.h>
#include <exceptions.h>

using namespace domotic_pi;

MqttSubscription::MqttSubscription(
	const std::string& host, 
	const int port, 
	std::string& topic, 
	std::function<void(const struct mosquitto_message *)> cb,
	const std::string& username,
	const std::string& password) : _cb(cb)
{
	// Allocate mosquitto structure for the new connection
	// Pointer to this object is stored inside the structure to be used during message callback
	mosq = mosquitto_new(NULL, true, this);
	if (!mosq) {
		console->error("MqttSubscription::ctor : module could not allocate memory for mqtt structure.");

		throw domotic_pi_exception("Connection to mqtt broker failed.");
	}

	// Set username and password if required
	if (!username.empty()) {
		mosquitto_username_pw_set(mosq, username.c_str(), password.c_str());
	}

	// Connect to the required mqtt broker
	int res = mosquitto_connect(mosq, host.c_str(), port, 60);
	if (res != MOSQ_ERR_SUCCESS) {
		console->error("MqttSubscription::ctor : module could not connect to borker at %s:%d : %s",
			host.c_str(), port, mosquitto_strerror(res));

		throw domotic_pi_exception("Connection to mqtt broker failed.");
	}

	// Subscribe to the required topic
	res = mosquitto_subscribe(mosq, NULL, topic.c_str(), 2);
	if (res != MOSQ_ERR_SUCCESS) {
		console->error("MqttSubscription::ctor : could not subscribe to '%s' : %s", 
			topic.c_str(), mosquitto_strerror(res));
		throw domotic_pi_exception("Could not subscribe to required topic");
	}
	mosquitto_message_callback_set(mosq, MqttSubscription::message_cb_router);

	// Start mqtt listener thread for the new connection
	res = mosquitto_loop_start(mosq);
	if (res != MOSQ_ERR_SUCCESS) {
		console->error("MqttSubscription::ctor : could not start mqtt client thread : %s", mosquitto_strerror(res));
		throw domotic_pi_exception("Thread start failed for mqtt listener.");
	}
}

MqttSubscription::~MqttSubscription()
{
	// Disconnect mqtt client and dispose listener thread
	bool forceStop = mosquitto_disconnect(mosq) != MOSQ_ERR_SUCCESS;
	mosquitto_loop_stop(mosq, forceStop);

	// Deallocate mosquitto structure for the closed connection
	mosquitto_destroy(mosq);
}

void MqttSubscription::message_cb_router(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	// Trigger message callback on relative object
	if (userdata) {
		((MqttSubscription*)userdata)->_cb(message);
	}
}