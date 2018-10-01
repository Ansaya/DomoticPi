#ifndef DOMOTIC_PI_INPUT
#define DOMOTIC_PI_INPUT

#include "CallbackToken.h"
#include "domoticPi.h"
#include "domoticPiDefine.h"
#include "IModule.h"
#include "ProgrammedEvent.h"

#include <functional>
#include <memory>
#include <rapidjson/document.h>
#ifdef DOMOTIC_PI_THREAD_SAFE
#include <shared_mutex>
#endif // DOMOTIC_PI_THREAD_SAFE
#include <tuple>
#include <type_traits>
#include <string>

namespace domotic_pi {

	class IInput : public IModule {

	public:

		/**
		 *	@brief Initialize a new input
		 *
		 *	@param id unique identifier for this input
		 */
		IInput(const std::string& id);

		IInput(const IInput&) = delete;
		IInput& operator= (const IInput&) = delete;
		virtual ~IInput();

		/**
		 *	@brief Get current input value
		 *
		 *	@return current input value
		 */
		virtual int getValue() const = 0;

		/**
		 *	@brief Register a callback to be triggered on value change
		 *
		 *	@note if trigger value is omitted, the event will be fired for every new value
		 * 
		 *	@param progEvent programmed event to trigger on value change
		 *	@param triggerValue value for which the programmed event will be triggered
		 */
		void addProgrammedEvent(ProgrammedEvent_ptr progEvent, int triggerValue = std::numeric_limits<int>::max());

		void removeProgrammedEvent(const std::string& programmedEventId);

		rapidjson::Document to_json() const override;

	protected:

		/**
		 *	@brief Initialize IInput attributes on an inheriting class object
		 *
		 *	@note This method must be called by the subclass implementing a from_json method
		 *		  just before returning the initialized subclass object
		 * 
		 *	@param config input module json configuration
		 *	@param input new input object initialized
		 *	@param parentNode input module parent node
		 * 
		 *	@return initialized input (same as given input)
		 */
		template<class T, typename = std::enable_if_t<std::is_base_of<IInput, T>::value>>
		static std::shared_ptr<T> from_json(
			const rapidjson::Value& config, 
			std::shared_ptr<T> input, 
			DomoticNode_ptr parentNode)
		{
			// Load programmed event bindings
			if (config.HasMember("triggerEvents")) {
				for (auto& it : config["triggerEvents"].GetArray()) {
					ProgrammedEvent_ptr pe = parentNode->getProgrammedEvent(it["eventId"].GetString());
					if (pe != nullptr) {
						// Place new programmed event-trigger value pair in the list
						input->_valueEventPairs.push_back(
							std::make_pair(
								it.HasMember("triggerValue") ? it["triggerValue"].GetInt() : std::numeric_limits<int>::max(), 
								std::weak_ptr<ProgrammedEvent>(pe)));

						console->debug("IInput::from_json : programmed event {} added to input {}.",
							pe->getID().c_str(), input->_id.c_str());
					}
					else {
						console->warn("IInput::from_json : programmed event {} not present in node {}.",
							pe->getID().c_str(), parentNode->getID().c_str());
					}
				}
			}

			return input;
		}

		/**
		 *	@brief Check for programmed events to be triggered and fires them if needed
		 *
		 *	@param newValue new value to check programmed events for
		 */
		void valueChanged(int newValue) const;

	private:
		std::shared_ptr<IInput> _this;
#ifdef DOMOTIC_PI_THREAD_SAFE
		mutable std::shared_mutex _valueEventPairsLock;
#endif // DOMOTIC_PI_THREAD_SAFE
		std::list<std::pair<int, std::weak_ptr<ProgrammedEvent>>> _valueEventPairs;

	};

	typedef std::shared_ptr<IInput> Input_ptr;

}

#endif