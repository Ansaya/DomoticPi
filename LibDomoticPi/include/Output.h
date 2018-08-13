#ifndef DOMOTIC_PI_OUTPUT
#define DOMOTIC_PI_OUTPUT

#include "domoticPiDefine.h"
#include "Module.h"
#include "OutState.h"
#include "Pin.h"

#ifdef DOMOTIC_PI_THREAD_SAFE
#include <mutex>
#endif // DOMOTIC_PI_THREAD_SAFE
#include <rapidjson/document.h>
#include <string>

namespace domotic_pi {

	class Output : 
		public Pin,
		public Module<Output>
	{

	public:
		/**
		 *	@brief Initialize base output structure
		 *
		 *	@param id output id
		 *	@param pinNumeber output pin number associated with the output
		 *
		 *	@note if no pin needs to be locked for the output just set any negative value
		 */
		Output(const std::string& id, int pinNumber);

		Output(const Output&) = delete;
		Output& operator= (const Output&) = delete;
		virtual ~Output();

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
		 *	@throws out_of_range if out of range pin numbers are required
		 */
		static Output_ptr from_json(const rapidjson::Value& config, 
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
		 *	@throws out_of_range if out of range pin numbers are required
		 */
		static Output_ptr from_json(const std::string& jsonConfig,
									DomoticNode_ptr parentNode);

		virtual int getValue() const;

		virtual void setState(OutState newState) = 0;

		virtual void setValue(int newValue) = 0;

	protected:
		int _value;
#ifdef DOMOTIC_PI_THREAD_SAFE
		std::mutex _valueLock;
#endif // DOMOTIC_PI_THREAD_SAFE

	};

}

#endif