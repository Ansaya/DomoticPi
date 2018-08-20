#include <IComm.h>

using namespace domotic_pi;

IComm::IComm(const std::string& id, const std::string& typeName) 
	: IModule(id), _type(typeName)
{
}

IComm::~IComm()
{
}

const std::string& IComm::getType() const
{
	return _type;
}

rapidjson::Document IComm::to_json() const
{
	rapidjson::Document comm = IModule::to_json();

	rapidjson::Value type;
	type.SetString(_type.c_str(), comm.GetAllocator());
	comm.AddMember("type", type, comm.GetAllocator());

	return comm;
}