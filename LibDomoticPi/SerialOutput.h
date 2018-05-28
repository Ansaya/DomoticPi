#ifndef DOMOTIC_PI_SERIAL_OUTPUT
#define DOMOTIC_PI_SERIAL_OUTPUT

#include "domoticPiDefine.h"
#include "Output.h"

#include <string>

namespace domotic_pi {

	class SerialOutput : public Output {

	public:
		SerialOutput(const std::string& id, SerialInterface_ptr serial, int min_range, int max_range);
		virtual ~SerialOutput();

		void setState(OutState newState) override;

		void setValue(int newValue) override;

		rapidjson::Document to_json() const override;

	private:
		SerialInterface_ptr _serial;
		int _range_min;
		int _range_max;
		int _value;

	};

}

#endif