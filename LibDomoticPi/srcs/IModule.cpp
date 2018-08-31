#include <IModule.h>

#include <domoticPi.h>

using namespace domotic_pi;

IModule::IModule(const std::string& id) 
	: _id(id), _name("")
{
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
	if (_nameInfo != nullptr) {
		_nameInfo->setValue(_name);
	}
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