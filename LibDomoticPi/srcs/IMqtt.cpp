#include <IMqtt.h>

#include <domoticPi.h>
#include <exceptions.h>

using namespace domotic_pi;

IMqtt::IMqtt(
	const std::string& id,
	const std::string& host,
	const int port,
	const std::string& username,
	const std::string& password) :
	_mosquittoLib(MqttLib::load()), _host(host), _port(port), _username(username), _password(password)
{
	// Allocate mosquitto structure for the new connection
	mosq = mosquitto_new(id.c_str(), true, NULL);
	if (!mosq) {
		console->error("MqttModule::ctor : module '{}' could not allocate memory for mqtt structure.",
			id.c_str());

		throw domotic_pi_exception("Connection to mqtt broker failed.");
	}

	// Set username and password if required
	if (!username.empty()) {
		mosquitto_username_pw_set(mosq, username.c_str(), password.c_str());
	}

	// Connect to the required mqtt broker
	int res = mosquitto_connect(mosq, host.c_str(), port, 60);
	if (res != MOSQ_ERR_SUCCESS) {
		console->error("MqttModule::ctor : module '{}' could not connect to borker at {}:{} : {}",
			id.c_str(), host.c_str(), port, mosquitto_strerror(res));

		throw domotic_pi_exception("Connection to mqtt broker failed.");
	}

	// Start mqtt listener thread for the new connection
	res = mosquitto_loop_start(mosq);
	if (res != MOSQ_ERR_SUCCESS) {
		console->error("MqttModule::ctor : could not start mqtt client thread for module '{}' : {}",
			id.c_str(), mosquitto_strerror(res));
		throw domotic_pi_exception("Thread start failed for mqtt listener.");
	}
}

IMqtt::~IMqtt()
{
	// Disconnect mqtt client and dispose listener thread
	bool forceStop = mosquitto_disconnect(mosq) != MOSQ_ERR_SUCCESS;
	mosquitto_loop_stop(mosq, forceStop);

	// Deallocate mosquitto structure for the closed connection
	mosquitto_destroy(mosq);
}

const std::string &IMqtt::getHost() const
{
	return _host;
}

int IMqtt::getPort() const
{
	return _port;
}

const std::string &IMqtt::getUsername() const
{
	return _username;
}

const std::string &IMqtt::getPassword() const
{
	return _password;
}

void IMqtt::publish(
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

	console->info("MqttModule::publish : message '{}' published on topic '{}'.",
		message.c_str(), topic.c_str());
}

MqttSubscription *IMqtt::subscribe(
	const std::string& topic,
	std::function<void(const struct mosquitto_message *)> message_cb)
{
	if (!_username.empty()) {
		return new MqttSubscription(_host, _port, topic, message_cb, _username, _password);
	}

	return new MqttSubscription(_host, _port, topic, message_cb);
}