#include <IOutput.h>

using namespace domotic_pi;

IOutput::IOutput(const std::string& id) 
	: IModule(id)
{
}

IOutput::~IOutput()
{
}

int IOutput::getValue() const
{
	return _value;
}