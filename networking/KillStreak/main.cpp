#include <iostream>
#include <stdlib.h>
#include "INIReader.h"
#include "ServerGame.hpp"
#include "ClientGame.h"
#include "logger.hpp"
#include "sysexits.h"


ServerGame * server = nullptr;
ClientGame * client = nullptr;


using namespace std;

int main(int argc, char** argv) {
	initLogging();
	auto log = logger();

	// Handle the command-line argument
	if (argc != 2) {
		cerr << "Usage: " << argv[0] << " [config_file]" << endl;
		return EX_USAGE;
	}

	// Read in the configuration file
	INIReader config(argv[1]);

	if (config.ParseError() < 0) {
		cerr << "Error parsing config file " << argv[1] << endl;
		return EX_CONFIG;
	}

	// NOTE: In the config file, if server is disabled, that means a client is running. Otherwise, the server is going to launch.
	if (config.GetBoolean("server", "enabled", true)) {
		log->info("Launching Killstreak server");
		server = new ServerGame(config);
		server->launch();
	}
	else {
		log->info("Launching Killstreak client");
		client = new ClientGame(config);
		client->run();
	}

	return 0;
}