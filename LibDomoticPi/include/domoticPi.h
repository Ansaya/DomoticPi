#ifndef DOMOTIC_PI
#define DOMOTIC_PI

#include <map>
#include <memory>
#include <rapidjson/schema.h>
#include <spdlog/spdlog.h>
#include <string>

namespace domotic_pi {

	extern std::shared_ptr<spdlog::logger> console;

	/**
	 *	@brief Set your own logger to be used by the library.
	 *
	 *	@param console pointer to spdlog logger to be used.
	 */
	void setConsole(std::shared_ptr<spdlog::logger> console);

	/**
	 *	@brief Initialize all needed stuff to run domoticPi library properly
	 *
	 *	@return 0 on success, non-zero on error
	 */
	int domoticPiInit();

	namespace json {

		class SchemaProvider : public rapidjson::IRemoteSchemaDocumentProvider {
		public:
			virtual const rapidjson::SchemaDocument* GetRemoteDocument(const char* uri, rapidjson::SizeType length);

			static int load();

			static const rapidjson::SchemaDocument* GetSchema(const char* uri);

		private:
			static std::map<std::string, std::shared_ptr<rapidjson::SchemaDocument>> _schemas;
		};

		bool hasValidSchema(const rapidjson::Value& json, const char* schemaUri);

	}

}

#endif