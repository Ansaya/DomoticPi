#include <IAHKAccessory.h>

using namespace domotic_pi;

IAHKAccessory::IAHKAccessory() : _ahkAccessory(nullptr)
{
}

IAHKAccessory::~IAHKAccessory()
{
}

bool IAHKAccessory::hasAHKAccessory() const 
{
	return _ahkAccessory != nullptr;
}

const hap::Accessory_ptr IAHKAccessory::getAHKAccessory() const
{
	return _ahkAccessory;
}