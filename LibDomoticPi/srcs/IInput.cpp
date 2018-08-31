#include <IInput.h>

#include <domoticPi.h>

#include <exception>

using namespace domotic_pi;

IInput::IInput(const std::string& id) 
	: IModule(id), _this(this), _valueChangeCallbacks()
{
}

IInput::~IInput() 
{
}

CallbackToken_ptr IInput::addValueChangeCallback(std::function<void(int)> callback)
{
	// When given function is nullptr, return a useless CallbackToken
	if (callback == nullptr) {
		console->debug("IInput::addValueChangeCallback : null callback added to input {}.", _id.c_str());
		return std::make_shared<CallbackToken>();
	}

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_valueChangeCBList);
#endif // DOMOTIC_PI_THREAD_SAFE

	_valueChangeCallbacks.push_back(std::make_tuple(_valueChangeCBCounter, callback));

	console->debug("IInput::addValueChangeCallback : value change callback added to input {}.", _id.c_str());

	auto weak_parent = std::weak_ptr<IInput>(_this);
	uint32_t cbIndex = _valueChangeCBCounter++;

	return std::make_shared<CallbackToken>([weak_parent, cbIndex] {
		if (weak_parent.expired()) {
			console->debug("IInput::__CallbackToken_lambda : parent expired before callback was removed.");
			return;
		}

		auto parent = weak_parent.lock();

#ifdef DOMOTIC_PI_THREAD_SAFE
		std::unique_lock<std::mutex> lock(parent->_valueChangeCBList);
#endif // DOMOTIC_PI_THREAD_SAFE

		for (auto cb = parent->_valueChangeCallbacks.begin(); cb != parent->_valueChangeCallbacks.end(); ++cb) {
			if (std::get<0>(*cb) == cbIndex) {
				parent->_valueChangeCallbacks.erase(cb);
				console->debug("IInput::__CallbackToken_lambda : callback {} removed "
					"from input {}", cbIndex, parent->_id.c_str());
				return;
			}
		}
	});
}

void IInput::valueChangeCallbacks(int newValue)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_valueChangeCBList);
#endif // DOMOTIC_PI_THREAD_SAFE

	for (auto& cbTuple : _valueChangeCallbacks) {
		try {
			std::get<1>(cbTuple)(newValue);
		}
		catch (std::exception& e) {
			console->warn("IInput::valueChangeCallbacks : exception during value "
				"change callback call from input {} : {}", _id.c_str(), e.what());
		}
	}
}