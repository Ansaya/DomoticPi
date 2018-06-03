#ifndef DOMOTIC_PI_DOMOTIC_API
#define DOMOTIC_PI_DOMOTIC_API

#include <cpprest/http_listener.h>
#include <string>

namespace domotic_pi {

namespace web {

class DomoticAPI {

public:
	DomoticAPI(std::string url);

	virtual ~DomoticAPI();

private:


};

}

}

#endif