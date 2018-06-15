#ifndef DOMOTIC_PI_EXCEPTIONS
#define DOMOTIC_PI_EXCEPTIONS

#include <exception>

namespace domotic_pi {

	class domotic_pi_exception : public std::exception {

	public:
		domotic_pi_exception(const char * message) : _message(message) {

		}

		virtual ~domotic_pi_exception() {}

		virtual const char * what() const throw()
		{
			return _message;
		}

	private:
		const char * _message;
	};

}

#endif