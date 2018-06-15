#ifndef DOMOTIC_PI_HAS_HAP_ACCESSORY
#define DOMOTIC_PI_HAS_HAP_ACCESSORY

#include "DomoticNode.h"

#include <hap/libHAP.h>
#include <memory>

namespace domotic_pi {

	class HasHAPAccessory {

	private:
		hap::Accessory_ptr getHAPAccessory() const
		{
			return _hapAccessory;
		}

		friend class DomoticNode;

	protected:
		const hap::Accessory_ptr _hapAccessory = std::make_shared<hap::Accessory>();

	};

}

#endif