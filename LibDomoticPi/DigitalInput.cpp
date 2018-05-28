#include "DigitalInput.h"

#include "domoticPi.h"
#include "exceptions.h"

#include <stdexcept>
#include <wiringPi.h>
#include <rapidjson/stringbuffer.h>

using namespace domotic_pi;

DigitalInput::DigitalInput(const std::string& id, int pinNumber, int pud) :
	Input(id, pinNumber), _pud(pud), _isr_mode(INT_EDGE_NONE)
{
	// Set required pull up/down mode
	pullUpDnControl(pinNumber, pud);

	console->info("DigitalInput::ctor : pin %d set as digital input with pud '%d'.", 
		pinNumber, pud);
}

DigitalInput::~DigitalInput()
{
	//	Disable interrupt if needed
	if (_isr_mode != INT_EDGE_NONE) {
		try {
			setISRMode(INT_EDGE_NONE);
		}
		catch (domotic_pi_exception& dpe) {
			console->error("DigitalInput::dtor : error clearing ISR calls : %s", dpe.what());
		}
	}
}

void DigitalInput::input_ISR()
{
	console->info("DigitalInput::input_ISR : ISR call execution for input '%s'.", getID().c_str());

	if (_isr_mode == INT_EDGE_NONE)
		return;

	// TODO: it could happen that a write to _isrActions happen during for iteration,
	//		 because no lock on _isrMode is acquired here. This is because it could 
	//		 lead to deadlock while disablig ISR when this function has started, but
	//		 has not yet acquired the lock.

	for (auto& action : _isrActions)
		action();
}

void DigitalInput::addISRCall(std::function<void()> cb, int isr_mode)
{
	if (cb == nullptr) {
		console->error("DigitalInput::addISRCall : setter called with null function pointer. "
				"(Input: '%s')", getID().c_str());
		throw std::invalid_argument("Callback function must not be null.");
	}

	if (isr_mode < 0 || isr_mode > 4) {
		console->error("DigitalInput::addISRCall : setter called with invalid isr mode '%d'. "
			"(Input: '%s')", getID().c_str());
		throw std::invalid_argument("isr_mode must be a value from 0 to 4");
	}

	// If requested isr mode is none, disable isr and complete the call
	if (isr_mode == INT_EDGE_NONE)
		return setISRMode(isr_mode);

	_setISRMode(isr_mode);

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_isrMode);
#endif
	_isrActions.push_back(cb);

	console->info("DigitalInput::addISRCall : ISR action added to input '%s'.", getID().c_str());
}

void DigitalInput::setISRMode(int isr_mode)
{
	console->info("DigitalInput::setISRMode : isr mode '%d' requested.", isr_mode);

	{
#ifdef DOMOTIC_PI_THREAD_SAFE
		std::unique_lock<std::mutex> lck(_isrMode);
#endif

		//	If no change is necessary, or function is called when no ISR function has been set
		//	do nothing
		if (_isr_mode == isr_mode || (isr_mode != INT_EDGE_NONE && _isrActions.empty()))
			return;
	}

	_setISRMode(isr_mode);
}

void DigitalInput::_setISRMode(int isr_mode) 
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_isrMode);
#endif

	int retval;

	// When switching from disabled to enabled bind instance function
	if (_isr_mode == INT_EDGE_NONE)
		retval = wiringPiISR(getPin(), isr_mode, std::bind(&DigitalInput::input_ISR, this));
	else	// Else change isr mode only
		retval = wiringPiISR(getPin(), isr_mode, nullptr);

	if (retval) {
		console->error("DigitalInput::setISRMode : error while changing ISR mode for input '%s' "
			"(domotic_pi code: %d).", getID().c_str(), retval);
		throw domotic_pi_exception("ISR mode change failed.");
	}

	_isr_mode = isr_mode;

	//	When disabling ISR clear ISR actions vector
	if (_isr_mode == INT_EDGE_NONE)
		_isrActions.clear();

	console->info("DigitalInput::setISRMode : isr mode changed to '%d' for input '%s'.",
		_isr_mode, getID().c_str());
}

int DigitalInput::getValue() const
{
	return digitalRead(getPin());
}

rapidjson::Document DigitalInput::to_json() const
{
	rapidjson::Document input = Input::to_json();

	console->debug("DigitalInput::to_json : serializing input '%s'.", _id.c_str());

	input.AddMember("type", "digital", input.GetAllocator());

	input.AddMember("pin", _pin, input.GetAllocator());
	input.AddMember("pud", _pud, input.GetAllocator());
	input.AddMember("isr_mode", _isr_mode, input.GetAllocator());

	return input;
}