#include "sysexits.h"
#include <string>
#include "ServerGame.hpp"
#include "ServerNetwork.hpp"
#include "logger.hpp"

#define GAME_SIZE	2	// total players required to start game

ServerGame::ServerGame(INIReader& t_config) : config(t_config) {
	auto log = logger();

	// PULL VALUES FROM THE CONFIG FILE
	// pull the address and port for the server
	string servconf = config.Get("server", "host", "");
	if (servconf == "") {
		log->error("Host line not found in config file");
		exit(EX_CONFIG);
	}
	size_t idx = servconf.find(":");		// delimiter index
	if (idx == string::npos) {
		log->error("Config line {} is invalid", servconf);
		exit(EX_CONFIG);
	}
	
	// get host (config)
	string get_host = servconf.substr(0, idx);
	host = get_host.c_str();

	// get port (config)
	string get_port = servconf.substr(idx + 1);
	port = get_port.c_str();

	// get tick_rate (config)
	tick_rate = (double)(config.GetInteger("server", "tick_rate", -1));
	if (tick_rate == -1) {
		log->error("Invalid tick_rate in config file");
		exit(EX_CONFIG);
	}

//	client_id = 0;
	network = new ServerNetwork(host, port);
}


// TODO: Setup lobby, wait until all player connections are accepted
void ServerGame::launch() {
	auto log = logger();
	log->info("Game server live, waiting for {} players...", GAME_SIZE);

	// Accept connections until GAME_SIZE met
	unsigned int client_id = 0;		// THINK: Make this static in this script?	
	while (client_id < GAME_SIZE)
	{
		network->acceptNewClient(client_id);
		log->info("Waiting for {} players...", GAME_SIZE - client_id);
	}


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
			//update();
			delta--;
		}
	}
}


void ServerGame::update() {
	auto log = logger();
	log->info("Game server update");
}



