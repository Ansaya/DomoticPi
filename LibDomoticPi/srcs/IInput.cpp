#include <IInput.h>

using namespace domotic_pi;

IInput::IInput(const std::string& id, const std::string& moduleName) 
	: IModule(id, moduleName)
{
}

IInput::~IInput() 
{
}