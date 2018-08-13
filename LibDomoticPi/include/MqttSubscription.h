#ifndef DOMOTIC_PI_MQTT_SUBSCRIPTION
#define DOMOTIC_PI_MQTT_SUBSCRIPTION

#include <functional>
#include <mosquitto.h>

namespace domotic_pi {

class MqttSubscription {
public:
	~MqttSubscription();

	MqttSubscription(const MqttSubscription&) = delete;
	MqttSubscription& operator= (const MqttSubscription&) = delete;

private:
	MqttSubscription(
		const std::string& host, 
		const int port, 
		const std::string& topic, 
		std::function<void(const struct mosquitto_message *)> cb,
		const std::string& username = "",
		const std::string& password = "");

	static void message_cb_router(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);

	struct mosquitto *mosq;
	std::function<void(const struct mosquitto_message *)> _cb;

	friend class IMqtt;
};

}

#endif // !DOMOTIC_PI_MQTT_SUBSCRIPTION
