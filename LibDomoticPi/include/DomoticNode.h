#ifndef DOMOTIC_PI_DOMOTIC_NODE
#define DOMOTIC_PI_DOMOTIC_NODE

#include "domoticPiDefine.h"

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

		const std::string & getName() const;

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
		 *		Serial interfaces handlers
		 */
#pragma region SerialInterfaces

		SerialInterface_ptr getSerialInterface(const std::string& id) const;

		/**
		 *	@brief Get full list of serial interfaces in this node
		 *	
		 *	@return Available serial interfaces' vector
		 */
		const std::vector<SerialInterface_ptr> & getSerialInterfaces() const;

		/**
		 *	@brief Add a new serial interface to the node
		 *
		 *	@param serialInterface serial interface to add to this node
		 *
		 *	@return True if operation succeded, false if a serial interface with same port was already present
		 */
		bool addSerialInterface(SerialInterface_ptr serialInterface);

		/**
		 *	@brief Remove serial interface with given port from the node if present
		 *
		 *	@param serialInterfacePort port of the serial interface module to remove
		 */
		void removeSerialInterface(const std::string& serialInterfacePort);

#pragma endregion

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		bool enableHAP();

		void disableHAP();

		bool isHAPEnabled() const;
#endif

	private:
		const std::string _id;
		std::string _name;
		std::vector<Input_ptr> _inputs;
		std::vector<Output_ptr> _outputs;
		std::vector<SerialInterface_ptr> _serialPorts;

#ifdef DOMOTIC_PI_THREAD_SAFE
		std::mutex _nameLock;
		std::mutex _inputsLock;
		std::mutex _outputsLock;
		std::mutex _serialInterfacesLock;
#endif // DOMOTIC_PI_THREAD_SAFE

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		
#endif

	};

}

#endif