#ifndef DOMOTIC_PI_COMM_FACTORY
#define DOMOTIC_PI_COMM_FACTORY

#include "domoticPiDefine.h"
#include "IComm.h"

#include <functional>
#include <map>
#ifdef DOMOTIC_PI_THREAD_SAFE
#include <mutex>
#endif // DOMOTIC_PI_THREAD_SAFE

#include <rapidjson/document.h>
#include <string>

namespace domotic_pi {

	class CommFactory {
	public:
		/**
		 *	@brief Deserialize and add new comm interface to parent node
		 *
		 *	@note If given serialized interface is already present in the parent node
		 *	it will be returned immediatley without trying to initialize a copy
		 *
		 *	@param config json configuration for a serial interface
		 *	@param parentNode parent node to associate the new interface to
		 *	@param checkSchema enable schema validation before parsing
		 *
		 *	@return new comm interface initialized from json configuration
		 *	@throws domotic_pi_exception for any error while parsing or initializing the object
		 */
		static Comm_ptr from_json(const rapidjson::Value& config,
			DomoticNode_ptr parentNode,
			bool checkSchema = false);

		/**
		 *	@brief Deserialize and add new comm interface to parent node
		 *
		 *	@note If given serialized interface is already present in the parent node
		 *	it will be returned immediatley without trying to initialize a copy
		 *
		 *	@note this method will always validate the json schema before parsing
		 *	the new object
		 *
		 *	@param jsonConfig json configuration string for a comm interface
		 *	@param parentNode parent node to associate the new interface to
		 *
		 *	@return new comm interface initialized from json configuration
		 *	@throws domotic_pi_exception for any error while parsing or initializing the object
		 */
		static Comm_ptr from_json(const std::string& jsonConfig,
			DomoticNode_ptr parentNode);

	protected:
		/**
		 *	@brief Register a type initializer for a comm class object
		 *
		 *	@note the registered function should only initialize the object from given json data,
		 *		  duplicate check is performed before initializer call and the returned object is
		 *		  added to parent node after the call
		 *
		 *	@param commType type string to check for in the json 'type' attribute
		 *	@param from_json initializer function to register for given type
		 *
		 *	@return always true
		 */
		static bool initializer_registration(
			const std::string& commType,
			std::function<Comm_ptr(const rapidjson::Value&, DomoticNode_ptr)> from_json);

	private:
#ifdef DOMOTIC_PI_THREAD_SAFE
		static std::mutex _commInitMap;
#endif // DOMOTIC_PI_THREAD_SAFE
		static std::map<std::string, std::function<Comm_ptr(const rapidjson::Value&, DomoticNode_ptr)>> _commInitializers;
	};

}

#endif // !DOMOTIC_PI_COMM_FACTORY
