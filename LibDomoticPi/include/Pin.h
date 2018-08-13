#ifndef DOMOTIC_PI_PIN
#define DOMOTIC_PI_PIN

#include "domoticPiDefine.h"

#include <memory>
#include <mutex>

namespace domotic_pi {

	//	This class is meant to be extended by any object willing to use a physical pin. 
	//	This is to avoid conflicts between different objects accessing the same pin 
	//	in different ways.
	class Pin {
	public:

		//	Tries to lock requested pin for exclusive use
		//
		//	@param pin Pin number to lock. (If no pin is needed any negative value is valid)
		//
		//	@throws out_of_range If pin number isn't in range 0-20
		//	@throws already_used_exception If pin is already used by another object
		Pin(int pin);

		Pin(const Pin&) = delete;
		Pin& operator= (const Pin&) = delete;

		virtual ~Pin();

		int getPin() const;

	protected:
		//	Object specific pin currently locked
		const int _pin;

	private:
		//	Global pin usage vector
		//	When a pin is locked and used _inUse[pinNumber] = true
		static bool _inUse [];
#ifdef DOMOTIC_PI_THREAD_SAFE
		static std::mutex _pinLock;
#endif

	};

}

#endif