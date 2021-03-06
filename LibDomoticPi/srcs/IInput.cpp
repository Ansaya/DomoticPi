#include <IInput.h>

#include <domoticPi.h>

#include <exception>

using namespace domotic_pi;

IInput::IInput(const std::string& id) 
	: IModule(id), _this(this), _valueEventPairs()
{
}

IInput::~IInput() 
{
}

void IInput::addProgrammedEvent(ProgrammedEvent_ptr progEvent, int triggerValue)
{
	// When given event is nullptr, return
	if (progEvent == nullptr) {
		return;
	}

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::shared_mutex> lock(_valueEventPairsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	// Remove previous programmed event occurrecies
	for (auto it = _valueEventPairs.begin(); it != _valueEventPairs.end(); ++it) {
		if (it->second.expired() || it->second.lock()->getID() == progEvent->getID()) {
			_valueEventPairs.erase(it);
		}
	}

	// Place new programmed event-trigger value pair in the list
	_valueEventPairs.push_back(std::make_pair(triggerValue, std::weak_ptr<ProgrammedEvent>(progEvent)));

	console->debug("IInput::addProgrammedEvent : programmed event {} added to input {}.", 
		progEvent->getID().c_str(), _id.c_str());
}

void IInput::removeProgrammedEvent(const std::string& programmedEventId)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::shared_mutex> lock(_valueEventPairsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	for (auto it = _valueEventPairs.begin(); it != _valueEventPairs.end(); ++it) {
		if (it->second.expired() || it->second.lock()->getID() == programmedEventId) {
			_valueEventPairs.erase(it);
			console->debug("IInput::removeProgrammedEvent : programmed event {} "
				"removed from input {}.", programmedEventId.c_str(), _id.c_str());
		}
	}
}

void IInput::valueChanged(int newValue) const
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::shared_lock<std::shared_mutex> lock(_valueEventPairsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	for (auto& valueEvent : _valueEventPairs) {
		try {
			if ((valueEvent.first == std::numeric_limits<int>::max() || valueEvent.first == newValue) 
				&& !valueEvent.second.expired()) {
				valueEvent.second.lock()->triggerEvent();
			}
		}
		catch (std::exception& e) {
			console->warn("IInput::valueChanged : exception during programmed "
				"event {} call from input {} : {}", 
				valueEvent.second.expired() ? "'removed event'" : valueEvent.second.lock()->getID().c_str(), 
				_id.c_str(), e.what());
		}
	}
}

rapidjson::Document IInput::to_json() const
{
	rapidjson::Document input = IModule::to_json();

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::shared_lock<std::shared_mutex> lock(_valueEventPairsLock);
#endif // DOMOTIC_PI_THREAD_SAFE

	rapidjson::Value triggerEvents(rapidjson::kArrayType);

	// Set not expired event triggers
	for (auto& it : _valueEventPairs) {

		if (!it.second.expired()) {
			rapidjson::Value valueEventPair(rapidjson::kObjectType);

			rapidjson::Value eventId;
			eventId.SetString(it.second.lock()->getID().c_str(), input.GetAllocator());
			input.AddMember("eventId", eventId, input.GetAllocator());

			if (it.first != std::numeric_limits<int>::max()) {
				rapidjson::Value triggerValue;
				triggerValue.SetInt(it.first);
				input.AddMember("triggerValue", triggerValue, input.GetAllocator());
			}

			triggerEvents.PushBack(valueEventPair, input.GetAllocator());
		}
	}

	input.AddMember("triggerEvents", triggerEvents, input.GetAllocator());

	return input;
}