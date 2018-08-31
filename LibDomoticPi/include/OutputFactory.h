#ifndef DOMOTIC_PI_OUTPUT_FACTORY
#define DOMOTIC_PI_OUTPUT_FACTORY

#include "domoticPiDefine.h"

#include <map>
#ifdef DOMOTIC_PI_THREAD_SAFE
#include <mutex>
#endif // DOMOTIC_PI_THREAD_SAFE
#include <rapidjson/document.h>
#include <string>

namespace domotic_pi {

	class OutputFactory {
	public:
		/**
		 *	@brief Deserialize and add new output to parent node
		 *
		 *	@note If given output object is already present in the parent node
		 *	it will be returned immediatley without trying to initialize a copy
		 *
		 *	@param jsonConfig json configuration string for an output
		 *	@param parentNode parent node to associate the new interface to
		 *
		 *	@return new output object initialized from json configuration
		 *	@throws domotic_pi_exception for any error while parsing or initializing the object
		 */
		static Output_ptr from_json(
			const rapidjson::Value& config,
			DomoticNode_ptr parentNode,
			bool checkSchema = false);

		/**
		 *	@brief Deserialize and add new output to parent node
		 *
		 *	@note If given output object is already present in the parent node
		 *	it will be returned immediatley without trying to initialize a copy
		 *
		 *	@note this method will always validate the json schema before parsing
		 *	the new object
		 *
		 *	@param jsonConfig json configuration string for an output
		 *	@param parentNode parent node to associate the new interface to
		 *
		 *	@return new output object initialized from json configuration
		 *	@throws domotic_pi_exception for any error while parsing or initializing the object
		 */
		static Output_ptr from_json(
			const std::string& jsonConfig,
			DomoticNode_ptr parentNode);

	protected:
		/**
		 *	@brief Register a type initializer for an output class object	
		 *
		 *	@note the registered function should only initialize the object from given json data,
		 *		  duplicate check is performed before initializer call and the returned object is
		 *		  added to parent node after the call
		 *
		 *	@param outputType type string to check for in the json 'type' attribute
		 *	@param from_json initializer function to register for given type
		 *
		 *	@return always true
		 */
		static bool initializer_registration(
			const std::string& outputType, 
			std::function<Output_ptr(const rapidjson::Value&, DomoticNode_ptr)> from_json);

	private:
#ifdef DOMOTIC_PI_THREAD_SAFE
		static std::mutex _outputInitMap;
#endif // DOMOTIC_PI_THREAD_SAFE
		static std::map<const std::string, std::function<Output_ptr(const rapidjson::Value&, DomoticNode_ptr)>> &_outputInitializers();
	};

}

#endif // !DOMOTIC_PI_OUTPUT_FACTORY
