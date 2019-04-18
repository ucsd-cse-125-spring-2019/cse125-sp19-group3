#include "sysexits.h"
#include <string>
#include "ServerGame.hpp"
#include "ServerNetwork.hpp"
#include "logger.hpp"

ServerGame::ServerGame(INIReader& t_config) : config(t_config) {
	auto log = logger();

	// PULL VALUES FROM THE CONFIG FILE
	// pull the address and port for the server
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
	port = servconf.substr(idx + 1).c_str();
	tick_rate = (double)(config.GetInteger("ssd", "tick_rate", -1));
	if (tick_rate == -1) {
		log->error("Invalid tick_rate in config file");
		exit(EX_CONFIG);
	}

	client_id = 0;
	network = new ServerNetwork(port);
}

void ServerGame::launch() {
	double ns = 1000000000.0 / tick_rate;
	double delta = 0;
	bool running = true; // not sure if needed;
	auto lastTime = Clock::now();

	// GAME LOOP
	while (running) {
		auto now = Clock::now();
		dsec ds = now - lastTime;
		nanoseconds duration = chrono::duration_cast<nanoseconds>(ds);
		delta += (duration.count() / ns);
		lastTime = now;

		while (delta >= 1) {
			update();
			delta--;
		}
	}
}

void ServerGame::update() {
	auto log = logger();
	log->info("Game server running");
}



