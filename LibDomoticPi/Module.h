#ifndef DOMOTIC_PI_MODULE
#define DOMOTIC_PI_MODULE

#include "domoticPi.h"
#include "domoticPiDefine.h"
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
#include "HasHAPAccessory.h"
#endif
#include "Serializable.h"

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
	public HasHAPAccessory 
#endif
{

public:
	Module(const std::string& id);

	const std::string& getID() const;

	const std::string& getName() const;

	void setName(const std::string& name);

	virtual rapidjson::Document to_json() const override;

	virtual hap::Accessory_ptr getHAPAccessory() const override;

protected:
	const std::string _id;

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