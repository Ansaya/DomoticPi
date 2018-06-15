#ifndef DOMOTIC_PI_DIGITAL_OUTPUT
#define DOMOTIC_PI_DIGITAL_OUTPUT

#include "Output.h"

#include <string>

namespace domotic_pi {

	class DigitalOutput : public Output {

	public:
		DigitalOutput(const std::string& id, int pinNumber);
		virtual ~DigitalOutput();
		
		void setState(OutState newState) override;

		void setValue(int newValue) override;

		rapidjson::Document to_json() const override;

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	private:
		hap::BoolCharacteristics_ptr _stateInfo;
#endif

	};

}

#endif
