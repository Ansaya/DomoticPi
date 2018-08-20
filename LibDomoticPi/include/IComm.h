#ifndef DOMOTIC_PI_ICOMM
#define DOMOTIC_PI_ICOMM

#include "IModule.h"

#include <functional>
#include <memory>
#include <string>

namespace domotic_pi {

	class IComm : public IModule {
	public:
		IComm(const std::string& id, const std::string& typeName);

		IComm(const IComm&) = delete;
		IComm& operator= (const IComm&) = delete;
		virtual ~IComm();

		/**
		 *	@brief Get comm type from derived class
		 */
		const std::string& getType() const;

		rapidjson::Document to_json() const override;

	private:
		const std::string _type;
	};

	typedef std::shared_ptr<IComm> Comm_ptr;

}

#endif // !DOMOTIC_PI_ICOMM
