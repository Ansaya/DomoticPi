#ifndef DOMOTIC_IMODULE
#define DOMOTIC_IMODULE

#include "domoticPiDefine.h"
#include "ModuleType.h"

#ifdef DOMOTIC_PI_APPLE_HOMEKIT
#include "IAHKAccessory.h"
#include <hap/libHAP.h>
#endif

#include <mutex>
#include <rapidjson/document.h>
#include <string>

namespace domotic {
	
namespace module {

	class IModule : 
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
		public IAHKAccessory 
#endif
	{

	public:
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
		virtual void setName(const std::string& name);


		const ModuleType& getType() const;

		rapidjson::Document to_json() const;

	protected:
		/**
		 *	@brief Initialize a new module with given unique identifier
		 *
		 *	@param id unique identifier for the new module
		 */
		IModule(const std::string& id, const ModuleType& type);

		const std::string _id;

	private:
		const ModuleType _type;
		std::string _name;
		std::mutex _nameLock;
	};

	typedef std::shared_ptr<IModule> Module_ptr;

}

}

#endif