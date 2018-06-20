#ifndef DOMOTIC_PI_MODULE
#define DOMOTIC_PI_MODULE

#include "domoticPi.h"
#include "domoticPiDefine.h"
#include "Serializable.h"

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
#include "IAHKAccessory.h"
#include <hap/libHAP.h>
#endif

#ifdef DOMOTIC_PI_THREAD_SAFE
#include <mutex>
#endif // DOMOTIC_PI_THREAD_SAFE
#include <rapidjson/document.h>
#include <string>

namespace domotic_pi {

template<class T>
class Module : 
	public Serializable,
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	public IAHKAccessory 
#endif
{

public:
	Module(const std::string& id);
	virtual ~Module();

	const std::string& getID() const;

	const std::string& getName() const;

	void setName(const std::string& name);

	virtual rapidjson::Document to_json() const override;

protected:
	const std::string _id;
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	hap::StringCharacteristics_ptr _nameInfo;
#endif

private:
	std::string _name;
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::mutex _nameLock;
#endif // DOMOTIC_PI_THREAD_SAFE
};

}

template<class T>
domotic_pi::Module<T>::Module(const std::string& id) : _id(id), _name("")
{
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_ahkAccessory = std::make_shared<hap::Accessory>();

	_ahkAccessory->addInfoService("Module", DOMOTIC_PI_APPLE_HOMEKIT_MANUFACTURER, 
		typeid(T).name(), _id, [](bool oldValue, bool newValue, void* sender) {});

	_nameInfo = std::make_shared<hap::StringCharacteristics>(hap::char_serviceName, hap::permission_read);
	_nameInfo->setValueChangeCB([this](std::string oldValue, std::string newValue, void* sender) {
		setName(newValue);
	});
#endif
}

template<class T>
domotic_pi::Module<T>::~Module()
{
}

template<class T>
const std::string & domotic_pi::Module<T>::getID() const
{
	return _id;
}

template<class T>
const std::string & domotic_pi::Module<T>::getName() const
{
	return _name;
}

template<class T>
void domotic_pi::Module<T>::setName(const std::string& name)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_nameLock);
#endif

	domotic_pi::console->info("Module::setName : name changed for module '%s' from '%s' to '%s'.",
		_id, _name.c_str(), name.c_str());

	_name.assign(name);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_nameInfo->Characteristics::setValue(_name);
#endif
}

template<class T>
rapidjson::Document domotic_pi::Module<T>::to_json() const
{
	domotic_pi::console->debug("Module::to_json : serializing module '%s'.", _id.c_str());

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

#endif