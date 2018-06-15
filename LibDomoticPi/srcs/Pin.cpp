#include <Pin.h>

#include <domoticPi.h>
#include <exceptions.h>

#include <stdexcept>
#include <wiringPi.h>

using namespace domotic_pi;

bool Pin::_inUse[DOMOTIC_PI_MAX_PIN];
#ifdef DOMOTIC_PI_THREAD_SAFE
std::mutex Pin::_pinLock;
#endif

Pin::Pin(int pin) : _pin(pin) {
	// When pin number is negative, no pin is requested
	if (pin < 0) 
		return;

	if (pin > DOMOTIC_PI_MAX_PIN) {
		console->error("Pin::ctor : pin %d not present.", pin);
		throw std::out_of_range("Pin number must be between 0 and " STR(DOMOTIC_PI_MAX_PIN) ".");
	}

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_pinLock);
#endif

	// If requested pin is already in use throw
	if (_inUse[pin]) {
		console->error("Pin::ctor : pin %d is already in use.", pin);
		throw domotic_pi_exception("Requested pin is alreay in use.");
	}

	// Else set pin in use and release lock
	_inUse[pin] = true;

	console->info("Pin::ctor : pin %d locked.", pin);
}

Pin::~Pin() {
	// When pin number is negative, no pin has been locked
	if (_pin < 0)
		return;

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_pinLock);
#endif

	// Reset pin status to not in use when destroying the object
	_inUse[_pin] = false;

	// Reset pin mode and pull to standard state
	pinMode(getPin(), DOMOTIC_PI_PIN_STANDARD_MODE);
	pullUpDnControl(getPin(), DOMOTIC_PI_PIN_STANDARD_PUD);

	console->info("Pin::dtor : pin %d unlocked.", _pin);
}

int Pin::getPin() const {
	return _pin;
}