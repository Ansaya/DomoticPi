#ifndef DOMOTIC_PI_DOMOTIC_NODE
#define DOMOTIC_PI_DOMOTIC_NODE

#include "domoticPiDefine.h"
#include "ProgrammedEvent.h"

#include <memory>
#ifdef DOMOTIC_PI_THREAD_SAFE
#include <mutex>
#endif
#include <rapidjson/document.h>
#include <string>
#include <vector>

namespace domotic_pi {

	class DomoticNode {
	public:
		DomoticNode(const std::string& id);

		DomoticNode(const DomoticNode&) = delete;
		DomoticNode& operator= (const DomoticNode&) = delete;
		virtual ~DomoticNode();

		static DomoticNode_ptr from_json(const rapidjson::Value& config, 
										 bool checkSchema = false);

		static DomoticNode_ptr from_json(const std::string& jsonConfig);

		const std::string & getID() const;

		void setName(const std::string& name);

		std::string getName() const;

		rapidjson::Document to_json() const;

		/**
		 *		Inputs handlers
		 */
#pragma region Inputs

		Input_ptr getInput(const std::string& id) const;

		/**
		 *	@brief Get full list of inputs in this node
		 *	
		 *	@return Available inputs' vector
		 */
		const std::vector<Input_ptr> & getInputs() const;

		/**
		 *	@brief Add a new input to the node
		 *	
		 *	@param input input to add to this node
		 *
		 *	@return True if operation succeded, false if an input with same id was already present
		 */
		bool addInput(Input_ptr input);

		/**
		 *	@brief Remove input with given id from the node if present
		 *	
		 *	@param inputId id of the input module to remove
		 */
		void removeInput(const std::string& inputId);

#pragma endregion

		/**
		 *		Outputs handlers
		 */
#pragma region Outputs

		Output_ptr getOutput(const std::string& id) const;

		/**
		 *	@brief Get full list of outputs in this node
		 *
		 *	@return Available outputs' vector
		 */
		const std::vector<Output_ptr> & getOutputs() const;

		/**
		 *	@brief Add a new output to the node
		 *
		 *	@param output output to add to this node
		 *
		 *	@return True if operation succeded, false if an output with same id was already present
		 */
		bool addOutput(Output_ptr output);

		/**
		 *	@brief Remove output with given id from the node if present
		 *
		 *	@param outputId id of the output module to remove
		 */
		void removeOutput(const std::string& outputId);

#pragma endregion

		/**
		 *		Comm interfaces handlers
		 */
#pragma region CommInterfaces

		Comm_ptr getComm(const std::string& id) const;

		/**
		 *	@brief Get full list of comm interfaces in this node
		 *	
		 *	@return Available comm interfaces' vector
		 */
		const std::vector<Comm_ptr> & getComms() const;

		/**
		 *	@brief Add a new comm interface to the node
		 *
		 *	@param comm serial interface to add to this node
		 *
		 *	@return True if operation succeded, false if a comm interface with same id was already present
		 */
		bool addComm(Comm_ptr comm);

		/**
		 *	@brief Remove comm interface with given port from the node if present
		 *
		 *	@param id id of the comm interface module to remove
		 */
		void removeComm(const std::string& id);

#pragma endregion

#pragma region ProgrammedEvents

		ProgrammedEvent_ptr getProgrammedEvent(const std::string& id) const;

		const std::vector<ProgrammedEvent_ptr> &getProgrammedEvents() const;

		bool addProgrammedEvent(ProgrammedEvent_ptr programmedEvent);

		void removeProgrammedEvent(const std::string& id);

#pragma endregion

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		/**
		 *	@brief Enable Apple HomeKit service for this node with given password and name
		 *
		 *	@note HAP service password should be formatted as XXX-XX-XXX where X is a 0-9 integer
		 *
		 *	@param password HAP service password for client connections
		 *	@param hapName HAP service name to expose, if omitted node id will be used
		 *
		 *	@return True if service has been correcty initialized, false else
		 */
		bool enableHAP(const std::string& password, const std::string& hapName = "");

		void disableHAP();

		bool isHAPEnabled() const;
#endif

	private:
		const std::string _id;
		std::string _name;
		std::vector<Input_ptr> _inputs;
		std::vector<Output_ptr> _outputs;
		std::vector<Comm_ptr> _comms;
		std::vector<ProgrammedEvent_ptr> _programmedEvents;

#ifdef DOMOTIC_PI_THREAD_SAFE
		mutable std::shared_mutex _nameLock;
		mutable std::shared_mutex _inputsLock;
		mutable std::shared_mutex _outputsLock;
		mutable std::shared_mutex _commsLock;
		mutable std::shared_mutex _programmedEventsLock;
#endif // DOMOTIC_PI_THREAD_SAFE

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		
#endif

	};

}

#endif