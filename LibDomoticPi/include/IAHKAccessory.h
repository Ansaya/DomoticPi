#ifndef DOMOTIC_PI_HAS_HAP_ACCESSORY
#define DOMOTIC_PI_HAS_HAP_ACCESSORY

#include "DomoticNode.h"

#include <hap/libHAP.h>
#include <memory>

namespace domotic_pi {

	class IAHKAccessory {

	private:
		const hap::Accessory_ptr getAHKAccessory() const
		{
			return _ahkAccessory;
		}

		friend class DomoticNode;

	protected:
		hap::Accessory_ptr _ahkAccessory;

	};

}

#endif