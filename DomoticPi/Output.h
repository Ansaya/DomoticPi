#ifndef DOMOTIC_PI_OUTPUT
#define DOMOTIC_PI_OUTPUT

#include "Pin.h"

#include <string>
#include <vector>

namespace domotic_pi {

	class Output : Pin {

	public:
		static Output fromJson(const std::string& jsonConfig);

		uint16_t getValue();

		void setValue(uint16_t new_value);

	protected:
		Output();
		virtual ~Output();

	private:
		uint8_t _pin;

	};

}

#endif