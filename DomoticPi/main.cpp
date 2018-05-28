#include "../LibDomoticPi/LibDomoticPi.h"
#include <cstdio>
#include <rapidjson/filereadstream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <spdlog/spdlog.h>

using namespace domotic_pi;

int main(int argc, char** argv)
{
	if (argc < 2) {
		printf("usage: %s <node_config_json>\n", argv[0]);
		return -1;
	}

	const std::string nodeConfigPath(argv[1]);

	auto console = spdlog::stdout_color_mt("main");

	domotic_pi::console->set_level(spdlog::level::debug);

	console->info("Hello from domotic pi main.");

	int retval = domoticPiInit(WPI_MODE_PINS);
	if (retval) {
		console->critical("DomoticPi setup went wrong.");
		return -1;
	}

	FILE* nodeConfigFile = fopen(nodeConfigPath.c_str(), "r");
	if (nodeConfigFile == nullptr) {
		console->critical("Could not open configuration file at '%s'.", nodeConfigPath.c_str());
		return -1;
	}

	char buffer[65535];
	rapidjson::FileReadStream is(nodeConfigFile, buffer, sizeof(buffer));

	rapidjson::Document configJson;
	configJson.ParseStream(is);
	fclose(nodeConfigFile);
	if (configJson.HasParseError()) {
		console->critical("Syntax error detected in given json file.");
		return -2;
	}

	DomoticNode_ptr localNode;
	try {
		localNode = DomoticNode::from_json(configJson, true);
	}
	catch (domotic_pi_exception& dpe) {
		console->critical("Exception while loading given node configuration: %s", dpe.what());

		return -3;
	}

	console->info("Domotic node loaded succesfully.");

	/*rapidjson::StringBuffer sb;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
	rapidjson::Document localNodeJson = localNode->to_json();
	localNodeJson.Accept(writer);

	console->info("Loaded domotic node json configuration: \n%s", sb.GetString());*/

	printf("Press a key to exit...");
	getchar();

	return 0;
}