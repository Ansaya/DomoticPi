#ifndef DOMOTIC_PI_SERIAL_OUTPUT
#define DOMOTIC_PI_SERIAL_OUTPUT

#include "domoticPiDefine.h"
#include "IOutput.h"
#include "OutputFactory.h"

#include <string>

namespace domotic_pi {

	class SerialOutput : public IOutput, protected OutputFactory {

	public:
		SerialOutput(const std::string& id, SerialInterface_ptr serial, int min_range, int max_range);

		SerialOutput(const SerialOutput&) = delete;
		SerialOutput& operator= (const SerialOutput&) = delete;
		virtual ~SerialOutput();

		void setState(OutState newState) override;

		void setValue(int newValue) override;

		rapidjson::Document to_json() const override;

	private:
		SerialInterface_ptr _serial;
		int _range_min;
		int _range_max;

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		hap::BoolCharacteristics_ptr _stateInfo;
		hap::IntCharacteristics_ptr _valueInfo;
#endif

		static const bool _factoryRegistration;
		static std::shared_ptr<SerialOutput> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);
	};

}

#endif