#include <MqttLib.h>

using namespace domotic_pi;

std::shared_ptr<MqttLib> MqttLib::_mosquitto;

MqttLib::MqttLib()
{
	mosquitto_lib_init();
}

MqttLib::~MqttLib()
{
	mosquitto_lib_cleanup();
}

const std::shared_ptr<MqttLib> MqttLib::load()
{
	if (_mosquitto == nullptr) {
		_mosquitto = std::make_shared<MqttLib>();
	}

	return _mosquitto;
}