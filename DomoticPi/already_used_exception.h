#ifndef DOMOTIC_PI_ALREADY_USED_EXCEPTION
#define DOMOTIC_PI_ALREADY_USED_EXCEPTION

#include <exception>

namespace domotic_pi {

	class already_used_exception : public std::exception {

		virtual const char* what() const throw() {
			return "Requested pin is already in use by another object.";
		}

	} already_used;

}

#endif