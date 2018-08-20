#ifndef DOMOTIC_PI_SERIALIZABLE
#define DOMOTIC_PI_SERIALIZABLE

#include <rapidjson/document.h>

namespace domotic_pi {

	/**
	 *	Through this interface the derived class exposes the possibility to store
	 *	its initialization attributes as a json object. Because of this an
	 *	appropriate factory should be implemented to restore the object fron json
	 *	state.
	 */
	class Serializable {
	public:
		virtual rapidjson::Document to_json() const = 0;
	};

}

#endif