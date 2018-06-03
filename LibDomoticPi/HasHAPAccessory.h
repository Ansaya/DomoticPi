#ifndef DOMOTIC_PI_HAS_HAP_ACCESSORY
#define DOMOTIC_PI_HAS_HAP_ACCESSORY

#include <libHAP.h>

namespace domotic_pi {

	class HasHAPAccessory {

	public:
		virtual hap::Accessory_ptr getHAPAccessory() const = 0;

	protected:
		hap::Accessory_ptr _hapAccessory;

	};

}

#endif