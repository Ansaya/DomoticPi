#include <MqttVolume.h>

#include <CommFactory.h>
#include <domoticPi.h>
#include <exceptions.h>
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
#include <hap/libHAP.h>
#endif // DOMOTIC_PI_APPLE_HOMEKIT

using namespace domotic_pi;

const bool MqttVolume::_factoryRegistration = 
	OutputFactory::initializer_registration("MqttVolume", MqttVolume::from_json);

MqttVolume::MqttVolume(
	const std::string& id,
	const std::string& mqttVolumeTopic,
	std::shared_ptr<MqttComm> mqttComm)
	: IOutput(id, "VolumeOutput"), 
	_mqttComm(mqttComm), 
	_volCmndTopic("cmnd/" + mqttVolumeTopic),
	_volStatTopic("stat/" + mqttVolumeTopic),
	_volumeStat(mqttComm->subscribe(_volStatTopic, std::bind(&MqttVolume::_vol_stat_cb, this, std::placeholders::_1))),
	_range_min(0),
	_range_max(100)
{
	if (mqttComm == nullptr) {
		console->error("MqttVolume::ctor : given mqtt comm interface can not be null.");
		throw domotic_pi_exception("Mqtt comm interface can not be null.");
	}

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	hap::Service_ptr volumeService = std::make_shared<hap::Service>(hap::service_speaker);
	_ahkAccessory->addService(volumeService);

	volumeService->addCharacteristic(_nameInfo);

	_stateInfo = std::make_shared<hap::BoolCharacteristics>(hap::char_on, hap::permission_all);
	_stateInfo->Characteristics::setValue(std::to_string(false));
	_stateInfo->setValueChangeCB([this](bool oldValue, bool newValue, void* sender) {
		if (oldValue != newValue)
			setState(newValue ? ON : OFF);
	});
	volumeService->addCharacteristic(_stateInfo);

	_valueInfo = std::make_shared<hap::IntCharacteristics>(hap::char_outputVolume, hap::permission_all,
		_range_min, _range_max, 1, hap::unit_percentage);
	_valueInfo->hap::Characteristics::setValue(std::to_string(_value));
	_valueInfo->setValueChangeCB([this](int oldValue, int newValue, void* sender) {
		if (oldValue != newValue)
			setValue(newValue);
	});

	volumeService->addCharacteristic(_valueInfo);
#endif // DOMOTIC_PI_APPLE_HOMEKIT

	console->info("MqttVolume::ctor : module '{}' initialized as volume output.");
}

MqttVolume::~MqttVolume()
{
	delete _volumeStat;
}

void MqttVolume::setState(OutState newState)
{
	int newValue = newState == ON ? _range_max 
		: newState == OFF ? _range_min 
		: _value == _range_min ? _range_max
		: _range_min;

	return setValue(newValue);
}

void MqttVolume::setValue(int newValue)
{
	if (newValue < _range_min) {
		newValue = _range_min;
	}
	else if (newValue > _range_max) {
		newValue = _range_max;
	}

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_valueLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	_mqttComm->publish(_volCmndTopic, std::to_string(newValue));

	_value = newValue;

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_stateInfo->hap::Characteristics::setValue(std::to_string(_value != _range_min));
	_valueInfo->hap::Characteristics::setValue(std::to_string(_value));
#endif

	console->info("MqttVolume::setValue : output '{}' set to '{}'.", getID(), _value);
}

std::shared_ptr<MqttVolume> MqttVolume::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	const rapidjson::Value& mqttInterface = config["comm"];

	std::shared_ptr<MqttComm> mqttComm = nullptr;

	if (mqttInterface.IsString()) {
		mqttComm = std::dynamic_pointer_cast<MqttComm>(parentNode->getComm(mqttInterface.GetString()));
	}
	else {
		mqttComm = std::dynamic_pointer_cast<MqttComm>(CommFactory::from_json(mqttInterface, parentNode));
	}

	return std::make_shared<MqttVolume>(
		config["id"].GetString(),
		config["topic"].GetString(),
		mqttComm);
}

rapidjson::Document MqttVolume::to_json() const
{
	rapidjson::Document output = IOutput::to_json();

	console->debug("MqttVolume::to_json : serializing output '{}'.", _id.c_str());

	output.AddMember("type", "MqttOutput", output.GetAllocator());

	rapidjson::Value comm;
	comm.SetString(_mqttComm->getID().c_str(), output.GetAllocator());
	output.AddMember("comm", comm, output.GetAllocator());

	rapidjson::Value mqttTopic;
	mqttTopic.SetString(_volCmndTopic.substr(5).c_str(), output.GetAllocator());
	output.AddMember("mqttTopic", mqttTopic, output.GetAllocator());

	return output;
}

void MqttVolume::_vol_stat_cb(const struct mosquitto_message * message)
{
	// Read and lowercase received message
	std::string messageString((char *)message->payload, message->payloadlen);

	console->debug("MqttVolume::_vol_stat_cb : message '{}' from topic '{}'.",
		messageString.c_str(), _volCmndTopic.c_str());

	std::transform(messageString.begin(), messageString.end(), messageString.begin(), ::tolower);

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_valueLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	if (messageString.compare("on") == 0) {
		_value = _range_max;
	}
	else if (messageString.compare("off") == 0) {
		_value = _range_min;
	}
	else {
		_value = std::atoi(messageString.c_str());
	}

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_stateInfo->hap::Characteristics::setValue(std::to_string(_value != _range_min));
	_valueInfo->hap::Characteristics::setValue(std::to_string(_value));
#endif

	console->info("MqttVolume::_vol_stat_cb : output '{}' changed value to '{}'.", 
		getID().c_str(), _value);
}