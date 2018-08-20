#include <MqttComm.h>

#include <domoticPi.h>
#include <exceptions.h>

using namespace domotic_pi;

const bool MqttComm::_factoryRegistration = CommFactory::initializer_registration("MqttComm", MqttComm::from_json);

MqttComm::MqttComm(
	const std::string& host,
	const int port,
	const std::string& username,
	const std::string& password) :
	IComm(host, "MqttComm"),
	_mosquittoLib(MqttLib::load()), _host(host), _port(port), _username(username), _password(password)
{
	// Allocate mosquitto structure for the new connection
	mosq = mosquitto_new(NULL, true, NULL);
	if (!mosq) {
		console->error("MqttModule::ctor : mqtt comm for '{}' could not allocate memory for mqtt structure.",
			host.c_str());

		throw domotic_pi_exception("Connection to mqtt broker failed.");
	}

	// Set username and password if required
	if (!username.empty()) {
		mosquitto_username_pw_set(mosq, username.c_str(), password.c_str());
	}

	// Connect to the required mqtt broker
	int res = mosquitto_connect(mosq, host.c_str(), port, 60);
	if (res != MOSQ_ERR_SUCCESS) {
		console->error("MqttModule::ctor : mqtt comm for '{}' could not connect to borker at {}:{} : {}",
			host.c_str(), host.c_str(), port, mosquitto_strerror(res));

		throw domotic_pi_exception("Connection to mqtt broker failed.");
	}

	// Start mqtt listener thread for the new connection
	res = mosquitto_loop_start(mosq);
	if (res != MOSQ_ERR_SUCCESS) {
		console->error("MqttModule::ctor : could not start mqtt client thread for '{}' : {}",
			host.c_str(), mosquitto_strerror(res));
		throw domotic_pi_exception("Thread start failed for mqtt listener.");
	}
}

MqttComm::~MqttComm()
{
	// Disconnect mqtt client and dispose listener thread
	bool forceStop = mosquitto_disconnect(mosq) != MOSQ_ERR_SUCCESS;
	mosquitto_loop_stop(mosq, forceStop);

	// Deallocate mosquitto structure for the closed connection
	mosquitto_destroy(mosq);
}

const std::string &MqttComm::getHost() const
{
	return _host;
}

int MqttComm::getPort() const
{
	return _port;
}

const std::string &MqttComm::getUsername() const
{
	return _username;
}

const std::string &MqttComm::getPassword() const
{
	return _password;
}

void MqttComm::publish(
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

MqttSubscription *MqttComm::subscribe(
	const std::string& topic,
	std::function<void(const struct mosquitto_message *)> message_cb)
{
	if (!_username.empty()) {
		return new MqttSubscription(_host, _port, topic, message_cb, _username, _password);
	}

	return new MqttSubscription(_host, _port, topic, message_cb);
}

std::shared_ptr<MqttComm> MqttComm::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	if (config.HasMember("username")) {
		return std::make_shared<MqttComm>(
			config["broker"].GetString(),
			config["port"].GetInt(),
			config["username"].GetString(),
			config["password"].GetString());
	}
	else {
		return std::make_shared<MqttComm>(
			config["broker"].GetString(),
			config["port"].GetInt());
	}
}

rapidjson::Document MqttComm::to_json() const
{
	rapidjson::Document mqttComm = IComm::to_json();
	mqttComm.RemoveMember("id");

	console->debug("MqttComm::to_json : serializing mqtt interface '{}'.", _id.c_str());

	rapidjson::Value broker;
	rapidjson::Value port;
	broker.SetString(_host.c_str(), mqttComm.GetAllocator());
	port.SetInt(_port);
	mqttComm.AddMember("broker", broker, mqttComm.GetAllocator());
	mqttComm.AddMember("port", port, mqttComm.GetAllocator());

	if (!_username.empty()) {
		rapidjson::Value username;
		rapidjson::Value password;
		username.SetString(_username.c_str(), mqttComm.GetAllocator());
		password.SetString(_password.c_str(), mqttComm.GetAllocator());
		mqttComm.AddMember("username", username, mqttComm.GetAllocator());
		mqttComm.AddMember("password", password, mqttComm.GetAllocator());
	}

	return mqttComm;
}