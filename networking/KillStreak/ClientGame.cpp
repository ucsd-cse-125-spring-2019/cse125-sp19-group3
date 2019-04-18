#include "ClientGame.h"
#include "ClientNetwork.hpp"
#include "Logger.hpp"
#include "sysexits.h"

ClientGame::ClientGame(INIReader& t_config) : config(t_config) {
	auto log = logger();
	// PULL WHATEVER YOU NEED FROM THE CONFIG FILE HERE...
	string servconf = config.Get("ssd", "server", "");
	if (servconf == "") {
		log->error("Server line not found in config file");
		exit(EX_CONFIG);
	}
	size_t idx = servconf.find(":");
	if (idx == string::npos) {
		log->error("Config line {} is invalid", servconf);
		exit(EX_CONFIG);
	}

	// get port from config
	string get_port = servconf.substr(idx + 1);
	serverPort = get_port.c_str();
	
	network = new ClientNetwork(serverPort);
}

void ClientGame::run() {
	auto log = logger();
	while (true) {
		log->info("Client running");
	}
}