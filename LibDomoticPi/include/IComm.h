#ifndef DOMOTIC_PI_ICOMM
#define DOMOTIC_PI_ICOMM

#include "IModule.h"

#include <functional>
#include <memory>
#include <string>

namespace domotic_pi {

	class IComm : public IModule {
	public:
		/**
		 *	@brief Initialize a generic comm module
		 *
		 *	@param id unique identifier for this comm module
		 *	@param typeName name of derived comm type to be returned by getType method
		 */
		IComm(const std::string& id, const std::string& typeName);

		IComm(const IComm&) = delete;
		IComm& operator= (const IComm&) = delete;
		virtual ~IComm();

		/**
		 *	@brief Get comm type from derived class
		 *
		 *	@return typeName string passed during IComm initialization
		 */
		const std::string& getType() const;

		rapidjson::Document to_json() const override;

	private:
		const std::string _type;
	};

	typedef std::shared_ptr<IComm> Comm_ptr;

}

#endif // !DOMOTIC_PI_ICOMM
