#ifndef DOMOTIC_PI_IOUTPUT
#define DOMOTIC_PI_IOUTPUT

#include "domoticPiDefine.h"
#include "IModule.h"
#include "OutState.h"

#include <memory>
#include <mutex>
#include <string>

namespace domotic_pi {

	class IOutput : public IModule {
	public:
		/**
		 *	@brief Initialize a new output with given unique identifier and HAP module name
		 *
		 *	@param id unique identifier for the new module
		 *	@param moduleName module type name for HAK service
		 */
		IOutput(const std::string& id, const std::string& moduleName = "GenericOutput");

		IOutput(const IOutput&) = delete;
		IOutput& operator= (const IOutput&) = delete;
		virtual ~IOutput();

		/**
		 *	@brief Get current output value from the module
		 *
		 *	@return current output value
		 */
		virtual int getValue() const;

		/**
		 *	@brief Set output power state as on/off/toggle
		 *
		 *	@param newState state to move the output to
		 */
		virtual void setState(OutState newState) = 0;

		/**
		 *	@brief Set output to specified value
		 *
		 *	@param newValue value to set the output to
		 */
		virtual void setValue(int newValue) = 0;

	protected:
		int _value;
#ifdef DOMOTIC_PI_THREAD_SAFE
		std::mutex _valueLock;
#endif // DOMOTIC_PI_THREAD_SAFE

	};

	typedef std::shared_ptr<IOutput> Output_ptr;

}

#endif // !DOMOTIC_PI_IOUTPUT
