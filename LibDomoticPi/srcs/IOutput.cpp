#include <IOutput.h>

using namespace domotic_pi;

IOutput::IOutput(const std::string& id, const std::string& moduleName) 
	: IModule(id, moduleName)
{
}

IOutput::~IOutput()
{
}

int IOutput::getValue() const
{
	return _value;
}