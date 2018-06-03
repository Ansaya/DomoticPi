#ifndef DOMOTIC_PI_INPUT
#define DOMOTIC_PI_INPUT

#include "domoticPiDefine.h"
#include "Module.h"
#include "Pin.h"

#include <functional>
#include <memory>
#ifdef DOMOTIC_PI_THREAD_SAFE
#include <mutex>
#endif
#include <rapidjson/document.h>
#include <string>

namespace domotic_pi {

	class Input : public Pin, public Module<Input> {

	public:

		/**
		 *	@brief Initialize a new input
		 *
		 *	@note required pin number must not be already in use by another object
		 *
		 *	@param id unique identifier for this input
		 *	@param pinNumber pin number this input is associated to
		 *
		 *	@throw domotic_pi_exception if the pinNumber is already in use
		 *	@throw out_of_range if pinNumber is outside library boundaries
		 */
		Input(const std::string& id, int pinNumber);

		virtual ~Input();

		/**
		 *	@brief Deserialize and add new input to parent node
		 *
		 *	@note If given input object is already present in the parent node
		 *	it will be returned immediatley without trying to initialize a copy
		 *
		 *	@param jsonConfig json configuration string for an input
		 *	@param parentNode parent node to associate the new interface to
		 *
		 *	@return new input object initialized from json configuration
		 *	@throws domotic_pi_exception for any error while parsing or initializing the object
		 *	@throws out_of_range if out of range pin numbers are required
		 */
		static Input_ptr from_json(const rapidjson::Value& config, 
								  DomoticNode_ptr parentNode,
								  bool checkSchema = false);

		/**
		 *	@brief Deserialize and add new input to parent node
		 *
		 *	@note If given input object is already present in the parent node
		 *	it will be returned immediatley without trying to initialize a copy
		 *
		 *	@note this method will always validate the json schema before parsing 
		 *	the new object
		 *
		 *	@param jsonConfig json configuration string for an input
		 *	@param parentNode parent node to associate the new interface to
		 *
		 *	@return new input object initialized from json configuration
		 *	@throws domotic_pi_exception for any error while parsing or initializing the object
		 *	@throws out_of_range if out of range pin numbers are required
		 */
		static Input_ptr from_json(const std::string& jsonConfig,
								  DomoticNode_ptr parentNode);

		virtual int getValue() const = 0;

		/**
		 *	@brief Register a callback to be triggered on given ISR mode.
		 *
		 *	@note Specified ISR mode is unique and will be used for all previously
		 *	registered functions.
		 *
		 *	@param cb pointer to callback function to register
		 *	@param isr_mode isr mode to be used for all callbacks registered
		 *	
		 *	@throw domotic_pi_exception If something goes wrong while changing isr mode.
		 */
		virtual void addISRCall(std::function<void()> cb, int isr_mode) = 0;

		/**
		 *	@brief Change ISR trigger mode for this input
		 *
		 *	@param isr_mode ISR mode to use (if ISR is disabled related isr functions are cleared.)
		 *
		 *	@throw domotic_pi_exception	If something goes wrong in wiringPi
		 */
		virtual void setISRMode(int isr_mode) = 0;

		/**
		 *	@brief Serialize current object to json document
		 *
		 *	@return json document representation for current object
		 */
		virtual rapidjson::Document to_json() const;

	};

}

#endif