#ifndef DOMOTIC_PI_INPUT
#define DOMOTIC_PI_INPUT

#include "CallbackToken.h"
#include "domoticPiDefine.h"
#include "IModule.h"

#include <functional>
#include <memory>
#ifdef DOMOTIC_PI_THREAD_SAFE
#include <mutex>
#endif // DOMOTIC_PI_THREAD_SAFE
#include <string>
#include <tuple>

namespace domotic_pi {

	class IInput : public IModule {

	public:

		/**
		 *	@brief Initialize a new input
		 *
		 *	@param id unique identifier for this input
		 */
		IInput(const std::string& id);

		IInput(const IInput&) = delete;
		IInput& operator= (const IInput&) = delete;
		virtual ~IInput();

		/**
		 *	@brief Get current input value
		 *
		 *	@return current input value
		 */
		virtual int getValue() const = 0;

		/**
		 *	@brief Register a callback to be triggered on value change event
		 *
		 *	@param callback value change callback
		 */
		CallbackToken_ptr addValueChangeCallback(std::function<void(int)> callback);

	protected:
		/**
		 *	@brief Call each value change callback currently registered for this input
		 *
		 *	@param newValue new value to pass to the callback
		 */
		void valueChangeCallbacks(int newValue);

	private:
		std::shared_ptr<IInput> _this;
#ifdef DOMOTIC_PI_THREAD_SAFE
		std::mutex _valueChangeCBList;
#endif // DOMOTIC_PI_THREAD_SAFE
		uint32_t _valueChangeCBCounter;
		std::list<std::tuple<uint32_t, std::function<void(int)>>> _valueChangeCallbacks;

	};

	typedef std::shared_ptr<IInput> Input_ptr;

}

#endif