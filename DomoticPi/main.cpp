#include <cstdio>
#include <domoticPi/libDomoticPi.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <systemd/sd-daemon.h>

#include <mutex>
#include <condition_variable>
#include <unistd.h>

using namespace domotic_pi;

int main(int argc, char** argv)
{
	auto console = spdlog::stdout_color_mt("main");

	domotic_pi::console->set_level(spdlog::level::level_enum::debug);

	console->info("Hello from domotic pi service.");

	int retval = domoticPiInit();
	if (retval) {
		console->critical("DomoticPi setup went wrong.");
		return -1;
	}

	console->info("Loading configuration file...");

	char workingDir[PATH_MAX];
	if (getcwd(workingDir, sizeof(workingDir)) == NULL) {
		printf("getcwd error");
		sd_notifyf(0, "STATUS=Failed to get current working directory");
		return -1;
	}

	std::string nodeConfigPath(workingDir);
	nodeConfigPath.append("/nodeConfig.json");

	FILE* nodeConfigFile = fopen(nodeConfigPath.c_str(), "r");
	if (nodeConfigFile == nullptr) {
		console->critical("Could not open configuration file at '{}'.", nodeConfigPath.c_str());
		sd_notifyf(0, "STATUS=Failed to open configuration file: %s not found", nodeConfigPath.c_str());
		return -1;
	}

	char buffer[65535];
	rapidjson::FileReadStream is(nodeConfigFile, buffer, sizeof(buffer));

	rapidjson::Document configJson;
	configJson.ParseStream(is);
	fclose(nodeConfigFile);
	if (configJson.HasParseError()) {
		const char *parseErrorStr = rapidjson::GetParseError_En(configJson.GetParseError());
		console->critical("Syntax error detected in given json file : {}", parseErrorStr);
		sd_notifyf(0, "STATUS=Failed to parse configuration file: \n%s", parseErrorStr);
		return -2;
	}

	DomoticNode_ptr localNode;
	try {
		localNode = DomoticNode::from_json(configJson, true);
	}
	catch (domotic_pi_exception& dpe) {
		console->critical("Exception while loading given node configuration: {}", dpe.what());
		sd_notifyf(0, "STATUS=Exception thrown during execution : \n%s", dpe.what());
		return -3;
	}

	console->info("Domotic node loaded succesfully.");

	// Notify systemd for initialization completed
	sd_notify(0, "READY=1");

	std::mutex mWake;
	bool wake = false;
	std::condition_variable wakeCond;

	std::unique_lock<std::mutex> lock(mWake);
	wakeCond.wait(lock, [&wake] { return wake; });

	return 0;
}