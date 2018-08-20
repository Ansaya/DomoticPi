#include <IModule.h>

#include <domoticPi.h>

using namespace domotic_pi;

IModule::IModule(const std::string& id, const std::string& moduleName) 
	: _id(id)
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	, _hasAHKAccessory(!moduleName.empty())
#endif // DOMOTIC_PI_APPLE_HOMEKIT
	, _name("")
{
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	if (_hasAHKAccessory) {
		_ahkAccessory = std::make_shared<hap::Accessory>();

		_ahkAccessory->addInfoService("Module", DOMOTIC_PI_APPLE_HOMEKIT_MANUFACTURER,
			moduleName, _id, [](bool oldValue, bool newValue, void* sender) {});

		_nameInfo = std::make_shared<hap::StringCharacteristics>(hap::char_serviceName, hap::permission_read);
		_nameInfo->setValueChangeCB([this](std::string oldValue, std::string newValue, void* sender) {
			setName(newValue);
		});
	}
#endif
}

IModule::~IModule()
{
}

const std::string & IModule::getID() const
{
	return _id;
}

const std::string & IModule::getName() const
{
	return _name;
}

void IModule::setName(const std::string& name)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_nameLock);
#endif

	console->info("Module::setName : name changed for module '{}' from '{}' to '{}'.",
		_id, _name.c_str(), name.c_str());

	_name.assign(name);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_nameInfo->Characteristics::setValue(_name);
#endif
}

rapidjson::Document IModule::to_json() const
{
	console->debug("Module::to_json : serializing module '{}'.", _id.c_str());

	rapidjson::Document moduleDoc(rapidjson::kObjectType);

	// Set input id
	rapidjson::Value id;
	id.SetString(_id.c_str(), moduleDoc.GetAllocator());
	moduleDoc.AddMember("id", id, moduleDoc.GetAllocator());

	rapidjson::Value name;
	name.SetString(_name.c_str(), moduleDoc.GetAllocator());
	moduleDoc.AddMember("name", name, moduleDoc.GetAllocator());

	return moduleDoc;
}

bool IModule::hasAHKAccessory() const
{
	return _hasAHKAccessory;
}