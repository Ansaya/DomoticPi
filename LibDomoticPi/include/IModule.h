#ifndef DOMOTIC_PI_MODULE
#define DOMOTIC_PI_MODULE

#include "domoticPiDefine.h"
#include "Serializable.h"

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
#include "IAHKAccessory.h"
#include <hap/libHAP.h>
#endif

#ifdef DOMOTIC_PI_THREAD_SAFE
#include <mutex>
#endif // DOMOTIC_PI_THREAD_SAFE
#include <rapidjson/document.h>
#include <string>

namespace domotic_pi {

	class IModule;

	typedef std::shared_ptr<IModule> Module_ptr;

	class IModule : 
		public Serializable,
	#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		public IAHKAccessory 
	#endif
	{

	public:
		/**
		 *	@brief Initialize a new module with given unique identifier and HAP module name
		 *
		 *	@param id unique identifier for the new module
		 *	@param moduleName module type name for HAP service
		 */
		IModule(const std::string& id, const std::string& moduleName = "GenericModule");

		IModule(const IModule&) = delete;
		IModule& operator= (const IModule&) = delete;
		virtual ~IModule();

		/**
		 *	@brief Get unique identifier of this module
		 */
		const std::string& getID() const;

		/**
		 *	@brief Get current module name
		 */
		const std::string& getName() const;

		/**
		 *	@brief Update module name
		 *
		 *	@param name new name for this module
		 */
		void setName(const std::string& name);

		rapidjson::Document to_json() const override;

	protected:
		const std::string _id;
	#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		hap::StringCharacteristics_ptr _nameInfo;
	#endif

	private:
		std::string _name;
	#ifdef DOMOTIC_PI_THREAD_SAFE
		std::mutex _nameLock;
	#endif // DOMOTIC_PI_THREAD_SAFE
	};

}

#endif