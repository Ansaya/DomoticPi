#include <ProgrammedEvent.h>

#include <domoticPi.h>
#include <exceptions.h>

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
				std::make_pair(std::weak_ptr<IOutput>(output), it["outputValue"].GetInt()));
		}
		else {
			console->warn("ProgrammedEvent::from_json : output {} not found in node {}",
				it["outputId"].GetString(), parentNode->getID().c_str());
		}
	}

	// Add new programmed event to parent node
	parentNode->addProgrammedEvent(pe);

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
			it.first.lock()->setValue(it.second);
		}
	}
}

rapidjson::Document ProgrammedEvent::to_json() const
{
	rapidjson::Document programmedEvent(rapidjson::kObjectType);

	rapidjson::Value id;
	id.SetString(_id.c_str(), programmedEvent.GetAllocator());
	programmedEvent.AddMember("id", id, programmedEvent.GetAllocator());

#ifdef DOMOTIC_PI_THREAD_SAFE
	std::shared_lock<std::shared_mutex> lock(_outputValuePairsLock);
#endif

	rapidjson::Value outputActions(rapidjson::kArrayType);
	for (auto& it : _outputValuePairs) {
		if (!it.first.expired()) {
			auto output = it.first.lock();

			rapidjson::Value outputAction(rapidjson::kObjectType);
			rapidjson::Value outputId;
			rapidjson::Value outputValue;
			outputId.SetString(output->getID().c_str(), programmedEvent.GetAllocator());
			outputValue.SetInt(it.second);
			outputAction.AddMember("outputId", outputId, programmedEvent.GetAllocator());
			outputAction.AddMember("outputValue", outputValue, programmedEvent.GetAllocator());

			outputActions.PushBack(outputAction, programmedEvent.GetAllocator());
		}
	}

	programmedEvent.AddMember("outputActions", outputActions, programmedEvent.GetAllocator());

	return programmedEvent;
}