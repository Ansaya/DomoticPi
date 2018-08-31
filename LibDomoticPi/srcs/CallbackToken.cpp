#include <CallbackToken.h>

#include <domoticPi.h>

#include <exception>

using namespace domotic_pi;

CallbackToken::CallbackToken(std::function<void()> removeCallbackFunction) 
	: _removeCallbackFunction(removeCallbackFunction)
{
}

CallbackToken::~CallbackToken()
{
	if (_removeCallbackFunction != nullptr) {
		try {
			_removeCallbackFunction();
		}
		catch (std::exception& e) {
			console->debug("CallbackToken::dtor : exception during callback remove function call : {}", e.what());
		}
	}
}