#ifndef DOMOTIC_PI_DIGITAL_SWITCH
#define DOMOTIC_PI_DIGITAL_SWITCH

#include "IOutput.h"
#include "OutputFactory.h"
#include "Pin.h"

#include <string>

namespace domotic_pi {

	class DigitalSwitch : public Pin, public IOutput, protected OutputFactory {

	public:
		/**
		 *	@brief Initialize a new digital switch module using an onboard pin
		 *
		 *	@param id unique identifier for this output
		 *	@param pinNumber pin number to be used by this output
		 */
		DigitalSwitch(const std::string& id, int pinNumber);

		DigitalSwitch(const DigitalSwitch&) = delete;
		DigitalSwitch& operator= (const DigitalSwitch&) = delete;
		virtual ~DigitalSwitch();
		
		void setState(OutState newState) override;

		void setValue(int newValue) override;

		rapidjson::Document to_json() const override;

	private:
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		hap::BoolCharacteristics_ptr _stateInfo;
#endif

		static const bool _factoryRegistration;
		static std::shared_ptr<DigitalSwitch> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);

	};

}

#endif
