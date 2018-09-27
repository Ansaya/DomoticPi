#ifndef DOMOTIC_PI_PROGRAMMED_EVENT
#define DOMOTIC_PI_PROGRAMMED_EVENT

#include "domoticPiDefine.h"
#include "IOutput.h"
#include "Serializable.h"

#include <functional>
#include <list>
#include <memory>
#ifdef DOMOTIC_PI_THREAD_SAFE
#include <shared_mutex>
#endif
#include <tuple>
#include <string>

namespace domotic_pi {
	class ProgrammedEvent : public Serializable {
	public:

		/**
		 *	@brief Initialize a new programmed event with a unique identifier
		 * 
		 *	@param id unique identifier for this event
		 */
		ProgrammedEvent(const std::string& id);

		virtual ~ProgrammedEvent();

		/**
		 *	@brief Initialize a new ProgrammedEvent object from a json configuration
		 *
		 *	@param config json programmed event configuration
		 *	@param parentNode domotic node to initialize the programmed event for
		 *	@param checkSchema whether to check json schema correctness or not
		 * 
		 *	@throw domotic_pi_exception if json schema is not valid
		 */
		static std::shared_ptr<ProgrammedEvent> from_json(
			const rapidjson::Value& config, 
			DomoticNode_ptr parentNode,
			bool checkSchema = false);

		/**
		 *	@brief Get this programmed event unique identifier
		 */
		const std::string &getID() const;

		/**
		 *	@brief Add an output-value pair to be set on event trigger
		 * 
		 *	@note If an output with same id is already present in the list, it is
		 *		  substituted by the new one
		 * 
		 *	@param outputModule output module to change value of
		 *	@param newValue new value to set the output module to
		 */
		void addOutputAction(Output_ptr outputModule, int newValue);

		/**
		 *	@brief Remove an output module action from this programmed event
		 * 
		 *	@param outputId id of the output module to remove (if not present, nothing happens)
		 */
		void removeOutputAction(const std::string& outputId);

		/**
		 *	@brief Triggers all the actions currently present in this programmed event
		 */
		void triggerEvent() const;

		rapidjson::Document to_json() const override;

	private:
		const std::string _id;
#ifdef DOMOTIC_PI_THREAD_SAFE
		mutable std::shared_mutex _outputValuePairsLock;
#endif
		std::list<std::pair<std::weak_ptr<IOutput>, int>> _outputValuePairs;

	};

	typedef std::shared_ptr<ProgrammedEvent> ProgrammedEvent_ptr;
}


#endif