#ifndef DOMOTIC_IAHKACCESSORY
#define DOMOTIC_IAHKACCESSORY

#include <hap/libHAP.h>
#include <memory>

// TODO: fix this interface

namespace domotic {

	class IAHKAccessory {

	public:
		IAHKAccessory();

		virtual ~IAHKAccessory();

		/**
		 *	@brief Check if derived class has implemented an HomeKit accessory abstraction
		 *
		 *	@return True if the AHK accessory has been implemented, false else
		 */
		bool hasAHKAccessory() const;

	protected:
		hap::Accessory_ptr _ahkAccessory;

	private:
		const hap::Accessory_ptr getAHKAccessory() const;
	};

}

#endif