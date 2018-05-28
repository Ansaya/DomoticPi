#ifndef DOMOTIC_PI_SERIAL_INTERFACE
#define DOMOTIC_PI_SERIAL_INTERFACE

#include "domoticPiDefine.h"
#include "Pin.h"

#include <memory>
#include <mutex>
#include <rapidjson/document.h>
#include <string>

namespace domotic_pi {

	class SerialInterface {
	public:

		/**	
		 *	@brief Lock needed pins and initializes serial communication interface.
		 *
		 *	@param port Serial device port (ie. /dev/serial0)
		 *	@param speed Serial port baud rate
		 *	@param txPin Serial tx pin number
		 *	@param rxPin Serial rx pin number
		 *
		 *	@throws domotic_pi_exception if dedicated serial pins are already in use.
		 *	@throws out_of_range if given t/rx pins are out of library bounds
		 */
		SerialInterface(const std::string& port, int baud, int txPin, int rxPin);

		virtual ~SerialInterface();

		/**
		 *	@brief Deserialize and add new serial interface to parent node
		 *
		 *	@note If given serialized interface is already present in the parent node
		 *	it will be returned immediatley without trying to initialize a copy
		 *
		 *	@param config json configuration for a serial interface
		 *	@param parentNode parent node to associate the new interface to
		 *	@param checkSchema enable schema validation before parsing
		 *
		 *	@return new serial interface initialized from json configuration
		 *	@throws domotic_pi_exception for any error while parsing or initializing the object
		 *	@throws out_of_range if out of range pin numbers are required
		 */
		static SerialInterface_ptr from_json(const rapidjson::Value& config, 
											 DomoticNode_ptr parentNode, 
											 bool checkSchema = false);

		/**
		 *	@brief Deserialize and add new serial interface to parent node
		 *
		 *	@note If given serialized interface is already present in the parent node
		 *	it will be returned immediatley without trying to initialize a copy
		 *
		 *	@note this method will always validate the json schema before parsing 
		 *	the new object
		 *
		 *	@param jsonConfig json configuration string for a serial interface
		 *	@param parentNode parent node to associate the new interface to
		 *
		 *	@return new serial interface initialized from json configuration
		 *	@throws domotic_pi_exception for any error while parsing or initializing the object
		 *	@throws out_of_range if out of range pin numbers are required
		 */
		static SerialInterface_ptr from_json(const std::string& jsonConfig,
											 DomoticNode_ptr parentNode);

		/**
		 *	@breif Get device port used by this serial interface
		 *
		 *	@return Port name used by this seril interface
		 */
		const std::string & getPort() const;

		/**
		*	@breif Get device baud rate used by this serial interface
		*
		*	@return Baud rate used by this seril interface
		*/
		int getBaudRate() const;

		/**	
		 *	@brief Read all available data from serial interface if available.
		 *	
		 *	@note In case of read error or empty buffer an empty string is returned.
		 *
		 *	@return Message read from serial interface.
		 */
		std::string read();

		/**
		 *	@brief Writes given string to serial interface
		 *
		 *	@param message Message to write to serial interface.
		 */
		void write(const std::string& message);

		rapidjson::Document to_json() const;

	private:
		const std::string _port;
		const int _baudRate;
		Pin _pinTX;
		Pin _pinRX;
		int _serial;
#ifdef DOMOTIC_PI_THREAD_SAFE
		std::mutex _txOwn;
		std::mutex _rxOwn;
#endif

	};

}

#endif