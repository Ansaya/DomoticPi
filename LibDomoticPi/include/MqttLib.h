#ifndef DOMOTIC_PI_MQTT_LIB
#define DOMOTIC_PI_MQTT_LIB

#include <memory>
#include <mosquitto.h>

namespace domotic_pi {

	/*
	 *	@brief Wrapper class to be used as a loading token for mosquitto library
	 */
	class MqttLib {
	public:
		MqttLib();

		MqttLib(const MqttLib&) = delete;
		MqttLib& operator= (const MqttLib&) = delete;
		~MqttLib();

		/*
		 *	@brief Call this method before using mosquitto library
		 *
		 *	This method ensure that mosquitto library is loaded after the call
		 *	and remains loaded until there is at least one active reference to
		 *	the returned object
		 *
		 *	@return mosquitto library load token
		 */
		static const std::shared_ptr<MqttLib> load();

	private:
		static std::shared_ptr<MqttLib> _mosquitto;
	};

}

#endif // !DOMOTIC_PI_MQTT_LIB
