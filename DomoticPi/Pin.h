#ifndef DOMOTIC_PI_PIN
#define DOMOTIC_PI_PIN

#include "already_used_exception.h"

#include <stdexcept>
#include <wiringPi.h>
#include <vector>

namespace domotic_pi {

	class Pin {
	public:
		Pin(uint8_t pin) : _pin(pin) {
			if (pin < 0 || pin > 20)
				throw std::out_of_range("Pin number must be between 0 and 20.");

			piLock(_lockKey);

			// If requested pin is already in use throw
			if (_inUse[pin]) {
				piUnlock(_lockKey);
				throw already_used;
			}

			// Else set pin in use and release lock
			_inUse[pin] = true;
			piUnlock(_lockKey);
		}

		virtual ~Pin() {
			// Reset pin status to not in use when destroying the object
			piLock(_lockKey);
			_inUse[_pin] = false;
			piUnlock(_lockKey);
		}

	private:
		static bool _inUse [20];
		static const uint8_t _lockKey = 0;

		uint8_t _pin;

	};

}

#endif