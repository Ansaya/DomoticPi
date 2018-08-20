#ifndef DOMOTIC_PI_INPUT
#define DOMOTIC_PI_INPUT

#include "domoticPiDefine.h"
#include "IModule.h"

#include <functional>
#include <memory>
#ifdef DOMOTIC_PI_THREAD_SAFE
#include <mutex>
#endif
#include <string>

namespace domotic_pi {

	class IInput : public IModule {

	public:

		/**
		 *	@brief Initialize a new input
		 *
		 *	@param id unique identifier for this input
		 *	@param moduleName module type name for HAK service
		 */
		IInput(const std::string& id, const std::string& moduleName = "GenericInput");

		IInput(const IInput&) = delete;
		IInput& operator= (const IInput&) = delete;
		virtual ~IInput();

		virtual int getValue() const = 0;

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
		virtual void addISRCall(std::function<void()> cb, int isr_mode) = 0;

		/**
		 *	@brief Change ISR trigger mode for this input
		 *
		 *	@param isr_mode ISR mode to use (if ISR is disabled related isr functions are cleared.)
		 *
		 *	@throw domotic_pi_exception	If something goes wrong in wiringPi
		 */
		virtual void setISRMode(int isr_mode) = 0;

	};

	typedef std::shared_ptr<IInput> Input_ptr;

}

#endif