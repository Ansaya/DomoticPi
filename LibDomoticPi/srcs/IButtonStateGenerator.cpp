#include <IButtonStateGenerator.h>

#include <domoticPi.h>

#include <exception>

using namespace domotic_pi;

IButtonStateGenerator::IButtonStateGenerator(
	const std::chrono::milliseconds doublePressDuration,
	const std::chrono::milliseconds longPressDuration)
	: _this(this),
	_doublePressDuration(doublePressDuration), _longPressDuration(longPressDuration),
	_doublePressCBCounter(0), _longPressCBCounter(0), 
	_doublePressRunning(false), _longPressRunning(false), 
	_doubleStateChange(false), _longStateChange(false), 
	_doublePressCheckThread(nullptr), _longPressCheckThread(nullptr)
{
}

IButtonStateGenerator::~IButtonStateGenerator()
{
	_doublePressRunning = false;
	_longPressRunning = false;

	_buttonStateChange.notify_all();

	if (_doublePressCheckThread != nullptr) {
		console->debug("IButtonStateGenerator::dtor : stopping double press check thread.");
		_doublePressCheckThread->join();
		delete _doublePressCheckThread;
	}

	if (_longPressCheckThread != nullptr) {
		console->debug("IButtonStateGenerator::dtor : stopping long press check thread.");
		_longPressCheckThread->join();
		delete _longPressCheckThread;
	}
}

std::tuple<std::chrono::milliseconds, std::chrono::milliseconds> IButtonStateGenerator::from_json(const rapidjson::Value& config)
{
	auto durations = std::make_tuple(std::chrono::milliseconds::zero(), std::chrono::milliseconds::zero());
	
	if (config.HasMember("doublePressDuration")) {
		std::get<0>(durations) = std::chrono::milliseconds(config["doublePressDuration"].GetInt());
	}

	if (config.HasMember("longPressDuration")) {
		std::get<1>(durations) = std::chrono::milliseconds(config["longPressDuration"].GetInt());
	}

	return durations;
}

void IButtonStateGenerator::buttonStateChange()
{
	std::unique_lock<std::mutex> lock(_stateChangeLock);
	_doubleStateChange = true;
	_longStateChange = true;
	_buttonStateChange.notify_all();
}

CallbackToken_ptr IButtonStateGenerator::addDoublePressCallback(std::function<void()> doublePressCallback)
{
	if (_doublePressDuration == std::chrono::milliseconds::zero() || doublePressCallback == nullptr) {
		return std::make_shared<CallbackToken>();
	}

	// If double press check thread is off, start it now
	if (_doublePressCheckThread == nullptr) {
		console->debug("IButtonStateGenerator::addDoublePressCallback : starting double press check thread.");
		_doublePressRunning = true;
		_doublePressCheckThread = new std::thread(&IButtonStateGenerator::_double_press_check, this);
	}

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_doublePressCBList);
#endif // DOMOTIC_PI_THREAD_SAFE

	_doublePressCallbacks.push_back(std::make_tuple(_doublePressCBCounter, doublePressCallback));

	console->debug("IButtonStateGenerator::addDoublePressCallback : callback added.");

	auto weak_parent = std::weak_ptr<IButtonStateGenerator>(_this);
	uint32_t cbIndex = _doublePressCBCounter++;

	return std::make_shared<CallbackToken>([weak_parent, cbIndex] {
		if (weak_parent.expired()) {
			console->debug("IButtonStateGenerator::__removeDoublePress_lambda : parent "
				"expired before callback was removed.");
			return;
		}

		auto parent = weak_parent.lock();
		
#ifdef DOMOTIC_PI_THREAD_SAFE
		std::unique_lock<std::mutex> lock(parent->_doublePressCBList);
#endif // DOMOTIC_PI_THREAD_SAFE
		for (auto cb = parent->_doublePressCallbacks.begin(); cb != parent->_doublePressCallbacks.end(); ++cb) {
			if (std::get<0>(*cb) == cbIndex) {
				parent->_doublePressCallbacks.erase(cb);
				console->debug("IButtonStateGenerator::__removeDoublePress_lambda : "
					"callback {} removed from parent.", cbIndex);
				break;
			}
		}

		// If double press callbacks list is empty, stop listener thread
		if (parent->_doublePressCallbacks.empty()) {
			console->debug("IButtonStateGenerator::__removeDoublePress_lambda : stopping double press check thread.");
			parent->_doublePressRunning = false;
			parent->_buttonStateChange.notify_all();
			parent->_doublePressCheckThread->join();
			delete parent->_doublePressCheckThread;
			parent->_doublePressCheckThread = nullptr;
		}
	});
}

CallbackToken_ptr IButtonStateGenerator::addLongPressCallback(std::function<void()> longPressCallback)
{
	if (_longPressDuration == std::chrono::milliseconds::zero() || longPressCallback == nullptr) {
		return std::make_shared<CallbackToken>();
	}

	// If long press check thread is off, start it now
	if (_longPressCheckThread == nullptr) {
		console->debug("IButtonStateGenerator::addLongPressCallback : starting long press check thread.");
		_longPressRunning = true;
		_longPressCheckThread = new std::thread(&IButtonStateGenerator::_long_press_check, this);
	}

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lock(_longPressCBList);
#endif // DOMOTIC_PI_THREAD_SAFE

	_longPressCallbacks.push_back(std::make_tuple(_longPressCBCounter, longPressCallback));

	console->debug("IButtonStateGenerator::addLongPressCallback : callback added.");

	auto weak_parent = std::weak_ptr<IButtonStateGenerator>(_this);
	uint32_t cbIndex = _longPressCBCounter++;

	return std::make_shared<CallbackToken>([weak_parent, cbIndex]() {
		if (weak_parent.expired()) {
			console->debug("IButtonStateGenerator::__removeLongPress_lambda : parent "
				"expired before callback was removed.");
			return;
		}

		auto parent = weak_parent.lock();
		
#ifdef DOMOTIC_PI_THREAD_SAFE
		std::unique_lock<std::mutex> lock(parent->_longPressCBList);
#endif // DOMOTIC_PI_THREAD_SAFE
		for (auto cb = parent->_longPressCallbacks.begin(); cb != parent->_longPressCallbacks.end(); ++cb) {
			if (std::get<0>(*cb) == cbIndex) {
				parent->_longPressCallbacks.erase(cb);
				console->debug("IButtonStateGenerator::__removeLongPress_lambda : "
					"callback {} removed from parent.", cbIndex);
				break;
			}
		}

		// If long press callbacks list is empty, stop listener thread
		if (parent->_longPressCallbacks.empty()) {
			console->debug("IButtonStateGenerator::__removeLongPress_lambda : stopping long press check thread.");
			parent->_longPressRunning = false;
			parent->_buttonStateChange.notify_all();
			parent->_longPressCheckThread->join();
			delete parent->_longPressCheckThread;
			parent->_longPressCheckThread = nullptr;
		}
	});
}

rapidjson::Document IButtonStateGenerator::to_json() const
{
	rapidjson::Document durationObject(rapidjson::kObjectType);

	if (_doublePressDuration != std::chrono::milliseconds::zero()) {
		durationObject.AddMember("doublePressDuration", _doublePressDuration.count(), durationObject.GetAllocator());
	}

	if (_longPressDuration != std::chrono::milliseconds::zero()) {
		durationObject.AddMember("longPressDuration", _longPressDuration.count(), durationObject.GetAllocator());
	}

	return durationObject;
}

void IButtonStateGenerator::_double_press_check()
{
	while (_doublePressRunning)
	{
		// Wait for first button change event
		std::unique_lock<std::mutex> lock(_stateChangeLock);
		while (!_doubleStateChange) {
			_buttonStateChange.wait(lock);
			if (!_doublePressRunning) {
				return;
			}
		}

		// State change flag reset
		_doubleStateChange = false;

		// Wait for the second button change event during the specified time duration 
		std::cv_status wakeStatus = _buttonStateChange.wait_for(lock, _doublePressDuration);

		// If button change event happens before the time duration expires, trigger relative callback
		if (wakeStatus != std::cv_status::timeout) {
			console->debug("IButtonStateGenerator::_double_press_check : double press event triggered.");
#ifdef DOMOTIC_PI_THREAD_SAFE
			std::unique_lock<std::mutex> cbLock(_doublePressCBList);
#endif // DOMOTIC_PI_THREAD_SAFE
			for (auto doublePressTuple : _doublePressCallbacks) {
				try {
					std::get<1>(doublePressTuple)();
				}
				catch (std::exception& e) {
					console->warn("IButtonStateGenerator::_double_press_check : exception "
						"during double press callback call : ", e.what());
				}
			}
		}
	}
}

void IButtonStateGenerator::_long_press_check()
{
	while (_longPressRunning)
	{
		// Wait for first button change event
		std::unique_lock<std::mutex> lock(_stateChangeLock);
		while (!_longStateChange) {
			_buttonStateChange.wait(lock);
			if (!_longPressRunning) {
				return;
			}
		}

		// State change flag reset
		_longStateChange = false;

		// Wait for the second button change event during the specified time duration 
		std::cv_status wakeStatus = _buttonStateChange.wait_for(lock, _longPressDuration);

		// If button change event doesn't happen during the long press time span, trigger relative callback
		if (wakeStatus == std::cv_status::timeout) {
			console->debug("IButtonStateGenerator::_long_press_check : long press event triggered.");
#ifdef DOMOTIC_PI_THREAD_SAFE
			std::unique_lock<std::mutex> cbLock(_longPressCBList);
#endif // DOMOTIC_PI_THREAD_SAFE
			for (auto longPressTuple : _longPressCallbacks) {
				try {
					std::get<1>(longPressTuple)();
				}
				catch (std::exception& e) {
					console->warn("IButtonStateGenerator::_long_press_check : exception "
						"during long press callback call : ", e.what());
				}
			}
		}
	}
}