#include <IModule/ModuleType.h>

using namespace domotic::module;

ModuleType::ModuleType(const std::string& family, const std::string& gender, const std::string& type)
	: _family(family), _gender(gender), _type(type)
{
}

ModuleType::~ModuleType()
{
}

bool ModuleType::operator==(const ModuleType& other)
{
	return _family == other._family && _gender == other._gender && _type == other._type;
}

const std::string& ModuleType::getFamily() const
{
	return _family;
}

const std::string& ModuleType::getGender() const
{
	return _gender;
}

const std::string& ModuleType::getType() const
{
	return _type;
}