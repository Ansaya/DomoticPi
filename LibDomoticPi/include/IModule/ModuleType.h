#ifndef DOMOTIC_MODULE_TYPE
#define DOMOTIC_MODULE_TYPE

#include <string>

namespace domotic {
	namespace module {
		class ModuleType {
			ModuleType(const std::string& family, const std::string& gender, const std::string& type);

			~ModuleType();

			bool operator==(const ModuleType& other);

			const std::string& getFamily() const;

			const std::string& getGender() const;

			const std::string& getType() const;

		private:
			const std::string _family;

			const std::string _gender;

			const std::string _type;
		};
	}
}

#endif