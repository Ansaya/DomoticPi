#include <ProgrammedEvent.h>

#include <DomoticNode.h>
#include <domoticPi.h>
#include <exceptions.h>
#include <IOutput.h>

using namespace domotic_pi;

ProgrammedEvent::ProgrammedEvent(const std::string& id) : _id(id)
{
}

ProgrammedEvent::~ProgrammedEvent()
{
	_outputValuePairs.clear();
}

std::shared_ptr<ProgrammedEvent> ProgrammedEvent::from_json(
	const rapidjson::Value& config,
	DomoticNode_ptr parentNode,
	bool checkSchema)
{
	if (checkSchema && !json::hasValidSchema(config, DOMOTIC_PI_JSON_PROGRAMMED_EVENT)) {
		console->error("ProgrammedEvent::fromJson : json schema not valid for a Programmed Event.");
		throw domotic_pi_exception("Json configuration for domotic_pi::ProgrammedEvent not valid.");
	}

	std::string id;
	id = config["id"].GetString();

	// Check if programmed event is already present to avoid duplication
	ProgrammedEvent_ptr pe = parentNode->getProgrammedEvent(id);
	if (pe != nullptr) {
		console->warn("ProgrammedEvent::from_json : programmed event {} already present in node {}.", 
			id.c_str(), parentNode->getID().c_str());
		return pe;
	}

	pe = std::make_shared<ProgrammedEvent>(id);

	// For each required action check if the corresponding output is present 
	// and add the output-value pair to actions list
	const rapidjson::Value::ConstArray& actions = config["outputActions"].GetArray();
	for (auto& it : actions) {
		Output_ptr output = parentNode->getOutput(it["outputId"].GetString());
		if (output != nullptr) {

			pe->_outputValuePairs.push_back(
				std::make_pair(
					std::weak_ptr<IOutput>(output), 
					it.HasMember("outputValue") ? it["outputValue"].GetInt() : std::numeric_limits<int>::max()));
		}
		else {
			console->warn("ProgrammedEvent::from_json : output {} not found in node {}",
				it["outputId"].GetString(), parentNode->getID().c_str());
		}
	}

	// Add new programmed event to parent node
	parentNode->addProgrammedEvent(pe);

	console->info("ProgrammedEvent::from_json : new programmed event created with id {} on node '{}'.", 
		id.c_str(), parentNode->getID().c_str());

	return pe;
}

const std::string &ProgrammedEvent::getID() const
{
	return _id;
}

void ProgrammedEvent::addOutputAction(Output_ptr outputModule, int newValue)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::shared_mutex> lock(_outputValuePairsLock);
#endif
	// If given output is already present erase it and insert the new occurence in the list
	// Meanwhile expired pointers are removed
	for (auto it = _outputValuePairs.begin(); it != _outputValuePairs.end(); ++it) {
		if (it->first.expired() || it->first.lock()->getID() == outputModule->getID()) {
			_outputValuePairs.erase(it);
		}
	}

	// Make and push the new output-value pair in the list
	_outputValuePairs.push_back(std::make_pair(std::weak_ptr<IOutput>(outputModule), newValue));

	console->debug("ProgrammedEvent::addOutputAction : action for output '{}' at value {} added to event ''.",
		outputModule->getID().c_str(), newValue, _id.c_str());
}

void ProgrammedEvent::removeOutputAction(const std::string& outputId)
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::unique_lock<std::shared_mutex> lock(_outputValuePairsLock);
#endif

	// Remove required value and check for expired pointer meanwhile
	for (auto it = _outputValuePairs.begin(); it != _outputValuePairs.end(); ++it) {
		if (it->first.expired() || it->first.lock()->getID() == outputId) {
			_outputValuePairs.erase(it);

			console->debug("ProgrammedEvent::removeOutputAction : action for output '{}' at value {} removed from event ''.",
				outputId.c_str(), it->second, _id.c_str());
		}
	}
}

void ProgrammedEvent::triggerEvent() const
{
#ifdef DOMOTIC_PI_THREAD_SAFE
	std::shared_lock<std::shared_mutex> lock(_outputValuePairsLock);
#endif

	// Set each output to stored value
	for (auto& it : _outputValuePairs) {
		if (!it.first.expired()) {
			if (it.second == std::numeric_limits<int>::max()) {
				it.first.lock()->setState(TOGGLE);
			}
			else {
				it.first.lock()->setValue(it.second);
			}
		}
	}
}

rapidjson::Document ProgrammedEvent::to_json() const
{
	rapidjson::Document programmedEvent(rapidjson::kObjectType);

	// Set event id
	rapidjson::Value id;
	id.SetString(_id.c_str(), programmedEvent.GetAllocator());
	programmedEvent.AddMember("id", id, programmedEvent.GetAllocator());

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::shared_lock<std::shared_mutex> lock(_outputValuePairsLock);
#endif

	// Set each output action from this event
	rapidjson::Value outputActions(rapidjson::kArrayType);
	for (auto& it : _outputValuePairs) {
		// Only still valid actions are set
		if (!it.first.expired()) {
			auto output = it.first.lock();

			rapidjson::Value outputAction(rapidjson::kObjectType);

			rapidjson::Value outputId;
			outputId.SetString(output->getID().c_str(), programmedEvent.GetAllocator());
			outputAction.AddMember("outputId", outputId, programmedEvent.GetAllocator());

			// Output value should be set only when necessary
			if (it.second != std::numeric_limits<int>::max()) {
				rapidjson::Value outputValue;
				outputValue.SetInt(it.second);
				outputAction.AddMember("outputValue", outputValue, programmedEvent.GetAllocator());
			}

			outputActions.PushBack(outputAction, programmedEvent.GetAllocator());
		}
	}

	programmedEvent.AddMember("outputActions", outputActions, programmedEvent.GetAllocator());

	return programmedEvent;
}