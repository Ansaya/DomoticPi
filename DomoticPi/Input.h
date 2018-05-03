#ifndef DOMOTIC_PI_INPUT
#define DOMOTIC_PI_INPUT

#include "Pin.h"
#include "Output.h"

#include <list>
#include <string>
#include <vector>

namespace domotic_pi {

	class Input : Pin {

	public:
		static Input fromJson(const std::string& jsonConfig);

		uint16_t getValue();

	protected:
		Input();
		virtual ~Input();

	private:
		uint8_t _pin;
		std::list<Output> _boundedOut;

		void input_ISR();

	};

}

#endif