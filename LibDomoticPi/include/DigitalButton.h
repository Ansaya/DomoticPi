#ifndef DOMOTIC_PI_DIGITAL_BUTTON
#define DOMOTIC_PI_DIGITAL_BUTTON

#include "domoticPiDefine.h"
#include "IButtonStateGenerator.h"
#include "IInput.h"
#include "InputFactory.h"
#include "Pin.h"

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace domotic_pi {

	class DigitalButton : 
		public Pin, 
		public IInput, 
		public IButtonStateGenerator,
		protected InputFactory {

	public:

		/**
		 *	@brief Initialize a new digital button
		 *
		 *	@note required pin number must not be already in use by another object
		 *
		 *	@param id unique identifier for this input button
		 *	@param pinNumber pin number this input is associated to
		 *	@param pud pull up/down state required by this input
		 *	@param doublePressDuration time duration within a double state change is translated to a double press event
		 *	@param longPressDuration time duration current state has to be maintained to trigger a long press event
		 *
		 *	@throw domotic_pi_exception if the pinNumber is already in use
		 *	@throw out_of_range if pinNumber is outside library boundaries
		 */
		DigitalButton(
			const std::string& id, 
			int pinNumber, 
			int pud,
			const std::chrono::milliseconds doublePressDuration = std::chrono::milliseconds::zero(),
			const std::chrono::milliseconds longPressDuration = std::chrono::milliseconds::zero());

		DigitalButton(const DigitalButton&) = delete;
		DigitalButton& operator= (const DigitalButton&) = delete;
		virtual ~DigitalButton();

		int getValue() const override;

		/**
		 *	@brief Change ISR trigger mode for this input
		 *
		 *	@param isr_mode ISR mode to use (if ISR is disabled related isr functions are cleared.)
		 *
		 *	@throw domotic_pi_exception	If something goes wrong in wiringPi
		 */
		void setISRMode(int isr_mode);

		/**
		 *	@brief Serialize current object to json document
		 *
		 *	@return json document representation for current object
		 */
		rapidjson::Document to_json() const override;

	private:
		const int _pud;
		int _isr_mode;

#ifdef DOMOTIC_PI_THREAD_SAFE
		std::mutex _isrMode;
#endif // DOMOTIC_PI_THREAD_SAFE

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		hap::IntCharacteristics_ptr _stateInfo;
		CallbackToken_ptr _doublePressToken;
		CallbackToken_ptr _longPressToken;
#endif // DOMOTIC_PI_APPLE_HOMEKIT

		/**
		 *	@brief Object related function binded to ISR for this input.
		 *
		 *	This function will call all the registered callbacks on ISR trigger.
		 */
		void input_ISR();

		static const bool _factoryRegistration;
		static std::shared_ptr<DigitalButton> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);

	};

}

#endif