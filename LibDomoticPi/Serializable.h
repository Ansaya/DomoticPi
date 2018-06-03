#ifndef DOMOTIC_PI_SERIALIZABLE
#define DOMOTIC_PI_SERIALIZABLE

#include <rapidjson/document.h>

namespace domotic_pi {

	class Serializable {
	public:
		virtual rapidjson::Document to_json() const = 0;
	};

}

#endif