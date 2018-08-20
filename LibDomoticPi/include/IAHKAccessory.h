#ifndef DOMOTIC_PI_HAS_HAP_ACCESSORY
#define DOMOTIC_PI_HAS_HAP_ACCESSORY

#include "DomoticNode.h"

#include <hap/libHAP.h>
#include <memory>

namespace domotic_pi {

	class IAHKAccessory {

	public:
		IAHKAccessory();

		virtual ~IAHKAccessory();

		/**
		 *	@brief Check if derived class has implemented an HomeKit accessory abstraction
		 *
		 *	@note This method will always return false, if the AHK accessory is implemented
		 *	      from a derived class this method shoul de overridden and return true.
		 *
		 *	@return True if the AHK accessory has been implemented, false else
		 */
		virtual bool hasAHKAccessory() const;

	protected:
		hap::Accessory_ptr _ahkAccessory;

	private:
		const hap::Accessory_ptr getAHKAccessory() const;

		friend class DomoticNode;
	};

}

#endif