#include <DigitalInput.h>

#include <domoticPi.h>
#include <exceptions.h>

#include <exception>
#include <rapidjson/stringbuffer.h>
#include <stdexcept>
#include <wiringPi.h>

using namespace domotic_pi;

const bool DigitalInput::_factoryRegistration =
	InputFactory::initializer_registration("DigitalInput", DigitalInput::from_json);

DigitalInput::DigitalInput(const std::string& id, int pinNumber, int pud) :
	Pin(pinNumber), IInput(id), _pud(pud), _isr_mode(INT_EDGE_NONE)
{
	if (pinNumber < 0) {
		console->error("DigitalInput::ctor : pin number must be a valid pin for a digital input.");
		throw std::out_of_range("Pin number must be between 0 and " STR(DOMOTIC_PI_MAX_PIN) ".");
	}

	pinMode(pinNumber, INPUT);
	console->info("DigitalInput::ctor : pin {} set to INPUT mode.", pinNumber);

	// Set required pull up/down mode
	pullUpDnControl(pinNumber, pud);

	console->info("DigitalInput::ctor : pin {} set as digital input with pud '{}'.", 
		pinNumber, pud);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	hap::Service_ptr switchService = std::make_shared<hap::Service>(hap::service_switch);
	_ahkAccessory->addService(switchService);

	switchService->addCharacteristic(_nameInfo);

	_stateInfo = std::make_shared<hap::BoolCharacteristics>(hap::char_on, hap::permission_read);
	_stateInfo->Characteristics::setValue(std::to_string(getValue()));
	switchService->addCharacteristic(_stateInfo);
#endif
}

DigitalInput::~DigitalInput()
{
	//	Disable interrupt if needed
	if (_isr_mode != INT_EDGE_NONE) {
		try {
			setISRMode(INT_EDGE_NONE);
		}
		catch (domotic_pi_exception& dpe) {
			console->error("DigitalInput::dtor : error clearing ISR calls : {}", dpe.what());
		}
	}
}

void DigitalInput::input_ISR()
{
	console->info("DigitalInput::input_ISR : ISR call execution for input '{}'.", getID().c_str());

	if (_isr_mode == INT_EDGE_NONE)
		return;

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_stateInfo->hap::Characteristics::setValue(std::to_string(getValue()));
#endif // DOMOTIC_PI_APPLE_HOMEKIT

	// TODO: it could happen that a write to _isrActions happen during for iteration,
	//		 because no lock on _isrMode is acquired here. This is because it could 
	//		 lead to deadlock while disablig ISR when this function has started, but
	//		 has not yet acquired the lock.

	for (auto& action : _isrActions) {
		try {
			action();
		}
		catch (std::exception& e) {
			console->warn("DigitalInput::input_ISR : isr call throw the following exception : {}", e.what());
		}
	}
}

void DigitalInput::addISRCall(std::function<void()> cb, int isr_mode)
{
	if (cb == nullptr) {
		console->error("DigitalInput::addISRCall : setter called with null function pointer. "
				"(Input: '{}')", getID().c_str());
		throw std::invalid_argument("Callback function must not be null.");
	}

	if (isr_mode < 0 || isr_mode > 4) {
		console->error("DigitalInput::addISRCall : setter called with invalid isr mode '{}'. "
			"(Input: '{}')", getID().c_str());
		throw std::invalid_argument("isr_mode must be a value from 0 to 4");
	}

	// If requested isr mode is none, disable isr and complete the call
	if (isr_mode == INT_EDGE_NONE)
		return setISRMode(isr_mode);

	_setISRMode(isr_mode);

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_isrMode);
#endif
	_isrActions.push_back(cb);

	console->info("DigitalInput::addISRCall : ISR action added to input '{}'.", getID().c_str());
}

void DigitalInput::setISRMode(int isr_mode)
{
	console->info("DigitalInput::setISRMode : isr mode '{}' requested.", isr_mode);

	{
#ifdef DOMOTIC_PI_THREAD_SAFE
		std::unique_lock<std::mutex> lck(_isrMode);
#endif

		//	If no change is necessary, or function is called when no ISR function has been set
		//	do nothing
		if (_isr_mode == isr_mode || (isr_mode != INT_EDGE_NONE && _isrActions.empty()))
			return;
	}

	_setISRMode(isr_mode);
}

void DigitalInput::_setISRMode(int isr_mode) 
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_isrMode);
#endif

	int retval;

	// When switching from disabled to enabled bind instance function
	if (_isr_mode == INT_EDGE_NONE)
		retval = wiringPiISR(getPin(), isr_mode, std::bind(&DigitalInput::input_ISR, this));
	else	// Else change isr mode only
		retval = wiringPiISR(getPin(), isr_mode, nullptr);

	if (retval) {
		console->error("DigitalInput::setISRMode : error while changing ISR mode for input '{}' "
			"(domotic_pi code: {}).", getID().c_str(), retval);
		throw domotic_pi_exception("ISR mode change failed.");
	}

	_isr_mode = isr_mode;

	//	When disabling ISR clear ISR actions vector
	if (_isr_mode == INT_EDGE_NONE)
		_isrActions.clear();

	console->info("DigitalInput::setISRMode : isr mode changed to '{}' for input '{}'.",
		_isr_mode, getID().c_str());
}

int DigitalInput::getValue() const
{
	return digitalRead(getPin());
}

std::shared_ptr<DigitalInput> DigitalInput::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	return std::make_shared<DigitalInput>(
		config["id"].GetString(),
		config["pin"].GetInt(),
		config["pud"].GetInt());
}

rapidjson::Document DigitalInput::to_json() const
{
	rapidjson::Document input = IInput::to_json();

	console->debug("DigitalInput::to_json : serializing input '{}'.", _id.c_str());

	input.AddMember("type", "DigitalInput", input.GetAllocator());

	input.AddMember("pin", _pin, input.GetAllocator());
	input.AddMember("pud", _pud, input.GetAllocator());
	input.AddMember("isr_mode", _isr_mode, input.GetAllocator());

	return input;
}