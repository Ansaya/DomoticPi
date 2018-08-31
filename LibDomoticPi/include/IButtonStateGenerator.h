#ifndef DOMOTIC_PI_BUTTON_STATE_GENERATOR
#define DOMOTIC_PI_BUTTON_STATE_GENERATOR

#include "ButtonState.h"
#include "CallbackToken.h"
#include "domoticPiDefine.h"
#include "Serializable.h"

#include <chrono>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <rapidjson/document.h>
#include <thread>
#include <tuple>

namespace domotic_pi {

	class IButtonStateGenerator : public Serializable {
	public:
		/**
		 *	@brief Initialize a thread listener to trigger double press and long press events from button change information
		 *
		 *	@note Until callbacks are set through relative setter methods, event listener threads aren't started
		 *
		 *	@param doublePressDuration timeout within a double state change is translated to a double press event
		 *	@param longPressDuration time current state has to be maintained to trigger a long press event
		 *
		 *	@throw invalid_argument when doublePressDuration equals zero
		 */
		IButtonStateGenerator(
			const std::chrono::milliseconds doublePressDuration, 
			const std::chrono::milliseconds longPressDuration = std::chrono::milliseconds::zero());

		virtual ~IButtonStateGenerator();

		/**
		 *	@brief Get double and long press value from given json configration object
		 *
		 *	@param config configuration object containint 'doublePressDuration' and/or 'longPressDuration' attributes to load
		 *	
		 *	@return tuple containing loaded doublePressDuration and longPressDuration values
		 */
		static std::tuple<std::chrono::milliseconds, std::chrono::milliseconds> from_json(const rapidjson::Value& config);

		/**
		 *	@brief Set the double press event callback
		 *
		 *	@note The double press listener thread is started and stopped according to the presence of callbacks
		 *	
		 *	@param doublePressCallback double press event callback
		 *
		 *	@return callback token to keep while callback needs to be enabled, when the object is 
		 *			disposed the relative callback is not called any more
		 */
		CallbackToken_ptr addDoublePressCallback(std::function<void()> doublePressCallback);

		/**
		 *	@brief Set the long press event callback
		 *
		 *	@note The long press listener thread is started and stopped according to the presence of callbacks
		 *
		 *	@param longPressCallback long press event callback
		 *
		 *	@return callback token to keep while callback needs to be enabled, when the object is 
		 *			disposed the relative callback is not called any more
		 */
		CallbackToken_ptr addLongPressCallback(std::function<void()> longPressCallback);

		/**
		 *	@brief Serialize double and long press duration as json attributes of a json object
		 *
		 *	@return json object with doublePressDurtation and longPressDuration attributes
		 */
		rapidjson::Document to_json() const override;

	protected:
		/**
		 *	@brief Notify button state change event to the state generator
		 */
		void buttonStateChange();

	private:
		const std::shared_ptr<IButtonStateGenerator> _this;
		const std::chrono::milliseconds _doublePressDuration;
		const std::chrono::milliseconds _longPressDuration;

#ifdef DOMOTIC_PI_THREAD_SAFE
		std::mutex _doublePressCBList;
		std::mutex _longPressCBList;
#endif

		uint32_t _doublePressCBCounter;
		uint32_t _longPressCBCounter;
		std::list<std::tuple<uint32_t, std::function<void()>>> _doublePressCallbacks;
		std::list<std::tuple<uint32_t, std::function<void()>>> _longPressCallbacks;

		std::mutex _stateChangeLock;
		std::condition_variable _buttonStateChange;

		volatile bool _doublePressRunning;
		volatile bool _longPressRunning;
		volatile bool _doubleStateChange;
		volatile bool _longStateChange;
		std::thread * _doublePressCheckThread;
		std::thread * _longPressCheckThread;

		void _double_press_check();
		void _long_press_check();
	};

}

#endif // !DOMOTIC_PI_BUTTON_STATE
