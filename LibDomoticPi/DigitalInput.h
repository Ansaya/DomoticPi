#ifndef DOMOTIC_PI_DIGITAL_INPUT
#define DOMOTIC_PI_DIGITAL_INPUT

#include "domoticPiDefine.h"
#include "Input.h"

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace domotic_pi {

	class DigitalInput : public Input {

	public:

		/**
		*	@brief Initialize a new digital input
		*
		*	@note required pin number must not be already in use by another object
		*
		*	@param id unique identifier for this input
		*	@param pinNumber pin number this input is associated to
		*	@param pud pull up/down state required by this input
		*
		*	@throw domotic_pi_exception if the pinNumber is already in use
		*	@throw out_of_range if pinNumber is outside library boundaries
		*/
		DigitalInput(const std::string& id, int pinNumber, int pud);

		virtual ~DigitalInput();

		int getValue() const override;

		/**
		*	@brief Register a callback to be triggered on given ISR mode.
		*
		*	@note Specified ISR mode is unique and will be used for all previously
		*	registered functions.
		*
		*	@param cb pointer to callback function to register
		*	@param isr_mode isr mode to be used for all callbacks registered
		*
		*	@throw domotic_pi_exception If something goes wrong while changing isr mode.
		*/
		void addISRCall(std::function<void()> cb, int isr_mode) override;

		/**
		*	@brief Change ISR trigger mode for this input
		*
		*	@param isr_mode ISR mode to use (if ISR is disabled related isr functions are cleared.)
		*
		*	@throw domotic_pi_exception	If something goes wrong in wiringPi
		*/
		void setISRMode(int isr_mode) override;

		/**
		*	@brief Serialize current object to json document
		*
		*	@return json document representation for current object
		*/
		rapidjson::Document to_json() const override;

	private:
		const int _pud;
		int _isr_mode;
		std::vector<std::function<void()>> _isrActions;

#ifdef DOMOTIC_PI_THREAD_SAFE
		std::mutex _isrMode;
#endif // DOMOTIC_PI_THREAD_SAFE

		void _setISRMode(int isr_mode);

		/**
		*	@brief Object related function binded to ISR for this input.
		*
		*	This function will call all the registered callbacks on ISR trigger.
		*/
		void input_ISR();

	};

}

#endif