#include <DigitalButton.h>

#include <domoticPi.h>
#include <exceptions.h>

#include <exception>
#include <rapidjson/stringbuffer.h>
#include <stdexcept>
#include <wiringPi.h>

using namespace domotic_pi;

const bool DigitalButton::_factoryRegistration =
	InputFactory::initializer_registration("DigitalButton", DigitalButton::from_json);

DigitalButton::DigitalButton(
	const std::string& id, 
	int pinNumber, 
	int pud,
	const std::chrono::milliseconds doublePressDuration,
	const std::chrono::milliseconds longPressDuration) :
	Pin(pinNumber), 
	IInput(id), 
	IButtonStateGenerator(doublePressDuration, longPressDuration), 
	_pud(pud), _isr_mode(INT_EDGE_NONE)
{
	if (pinNumber < 0) {
		console->error("DigitalButton::ctor : pin number must be a valid pin for a digital input.");
		throw std::out_of_range("Pin number must be between 0 and " STR(DOMOTIC_PI_MAX_PIN) ".");
	}

	pinMode(pinNumber, INPUT);
	console->info("DigitalButton::ctor : pin {} set to INPUT mode.", pinNumber);

	// Set required pull up/down mode
	pullUpDnControl(pinNumber, pud);

	console->info("DigitalButton::ctor : pin {} set as digital input with pud '{}'.", 
		pinNumber, pud);

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_ahkAccessory = std::make_shared<hap::Accessory>();

	_ahkAccessory->addInfoService(getName(), DOMOTIC_PI_APPLE_HOMEKIT_MANUFACTURER,
		typeid(DigitalButton).name(), _id, "1.0.0", [](bool oldValue, bool newValue, void* sender) {});

	_ahkAccessory->addStatelessSwitchService(&_stateInfo, &_nameInfo, nullptr);

	_nameInfo->setValue(getName());

	_doublePressToken = addDoublePressCallback([=] {
		_stateInfo->setValue(double_press);
	});

	_longPressToken = addLongPressCallback([=] {
		_stateInfo->setValue(long_press);
	});
#endif
}

DigitalButton::~DigitalButton()
{
	//	Disable interrupt if needed
	try {
		setISRMode(INT_EDGE_NONE);
	}
	catch (domotic_pi_exception& dpe) {
		console->error("DigitalButton::dtor : error clearing ISR calls : {}", dpe.what());
	}
}

void DigitalButton::input_ISR()
{
	console->info("DigitalButton::input_ISR : ISR call execution for input '{}'.", getID().c_str());

	if (_isr_mode == INT_EDGE_NONE) {
		return;
	}

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
	_stateInfo->setValue(0);
#endif // DOMOTIC_PI_APPLE_HOMEKIT

	buttonStateChange();

	valueChangeCallbacks(_isr_mode == INT_EDGE_BOTH ? getValue() : _isr_mode == INT_EDGE_RISING ? 1 : 0);
}

void DigitalButton::setISRMode(int isr_mode)
{
	console->info("DigitalButton::setISRMode : isr mode '{}' requested.", isr_mode);

	//	If no change is necessary do nothing
	if (_isr_mode == isr_mode)
		return;

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::mutex> lck(_isrMode);
#endif

	int retval;

	// When switching from disabled to enabled bind instance function
	if (_isr_mode == INT_EDGE_NONE)
		retval = wiringPiISR(getPin(), isr_mode, std::bind(&DigitalButton::input_ISR, this));
	else	// Else change isr mode only
		retval = wiringPiISR(getPin(), isr_mode, nullptr);

	if (retval) {
		console->error("DigitalButton::setISRMode : error while changing ISR mode for input '{}' "
			"(domotic_pi code: {}).", getID().c_str(), retval);
		throw domotic_pi_exception("ISR mode change failed.");
	}

	_isr_mode = isr_mode;

	console->info("DigitalButton::setISRMode : isr mode changed to '{}' for input '{}'.",
		_isr_mode, getID().c_str());
}

int DigitalButton::getValue() const
{
	return digitalRead(getPin());
}

std::shared_ptr<DigitalButton> DigitalButton::from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode)
{
	auto durations = IButtonStateGenerator::from_json(config);

	auto digitalInput = std::make_shared<DigitalButton>(
		config["id"].GetString(),
		config["pin"].GetInt(),
		config["pud"].GetInt(),
		std::get<0>(durations),
		std::get<1>(durations));

	digitalInput->setISRMode(config["isr_mode"].GetInt());

	return digitalInput;
}

rapidjson::Document DigitalButton::to_json() const
{
	rapidjson::Document input = IInput::to_json();

	// Add double/long press duration from superclass
	rapidjson::Document stateDurations = IButtonStateGenerator::to_json();
	for (auto attr = stateDurations.MemberBegin(); attr != stateDurations.MemberEnd(); ++attr) {
		input.AddMember(attr->name, attr->value.Move(), input.GetAllocator());
	}

	console->debug("DigitalButton::to_json : serializing input '{}'.", _id.c_str());

	input.AddMember("type", "DigitalButton", input.GetAllocator());

	input.AddMember("pin", _pin, input.GetAllocator());
	input.AddMember("pud", _pud, input.GetAllocator());
	input.AddMember("isr_mode", _isr_mode, input.GetAllocator());

	return input;
}