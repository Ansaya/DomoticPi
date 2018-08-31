#ifndef DOMOTIC_PI_CALLBACK_TOKEN
#define DOMOTIC_PI_CALLBACK_TOKEN

#include <functional>
#include <memory>

namespace domotic_pi {

	class CallbackToken {
	public:
		/**
		 *	@brief Initialize a callback token which will call given function when destructed
		 *
		 *	@param removeCallbackFunction function to be called at object destruction
		 */
		CallbackToken(std::function<void()> removeCallbackFunction = nullptr);

		virtual ~CallbackToken();

	private:
		std::function<void()> _removeCallbackFunction;
	};

	typedef std::shared_ptr<CallbackToken> CallbackToken_ptr;

}

#endif // !DOMOTIC_PI_CALLBACK_TOKEN
