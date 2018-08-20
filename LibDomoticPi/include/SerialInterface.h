#ifndef DOMOTIC_PI_SERIAL_INTERFACE
#define DOMOTIC_PI_SERIAL_INTERFACE

#include "CommFactory.h"
#include "domoticPiDefine.h"
#include "IComm.h"
#include "Pin.h"

#include <memory>
#ifdef DOMOTIC_PI_THREAD_SAFE
#include <mutex>
#endif // DOMOTIC_PI_THREAD_SAFE
#include <rapidjson/document.h>
#include <string>

namespace domotic_pi {

	class SerialInterface : public IComm, protected CommFactory {
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

		SerialInterface(const SerialInterface&) = delete;
		SerialInterface& operator= (const SerialInterface&) = delete;
		virtual ~SerialInterface();

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

		rapidjson::Document to_json() const override;

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

		static const bool _factoryRegistration;
		static std::shared_ptr<SerialInterface> from_json(const rapidjson::Value& config, DomoticNode_ptr parentNode);

	};

}

#endif