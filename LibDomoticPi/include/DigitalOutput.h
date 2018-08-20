#ifndef DOMOTIC_PI_DIGITAL_OUTPUT
#define DOMOTIC_PI_DIGITAL_OUTPUT

#include "IOutput.h"
#include "OutputFactory.h"
#include "Pin.h"

#include <string>

namespace domotic_pi {

	class DigitalOutput : public Pin, public IOutput, protected OutputFactory {

	public:
		DigitalOutput(const std::string& id, int pinNumber);

		DigitalOutput(const DigitalOutput&) = delete;
		DigitalOutput& operator= (const DigitalOutput&) = delete;
		virtual ~DigitalOutput();
		
		void setState(OutState newState) override;

		void setValue(int newValue) override;

		rapidjson::Document to_json() const override;

	private:
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		hap::BoolCharacteristics_ptr _stateInfo;
#endif

		static const bool _factoryRegistration;
		static std::shared_ptr<DigitalOutput> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);

	};

}

#endif
