#include "logger.hpp"
#include "sysexits.h"
#include "INIReader.h"
#include "ClientGame.h"
#include "ServerGame.hpp"

#include <stdlib.h>
#include <iostream>
#include <process.h>
using namespace std;


ServerGame * server = nullptr;
ClientGame * client = nullptr;


// launch initialized server (called by new thread)
void run_server(void *arg)
{
	server->launch();
}


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
	while (1) {};
		return EX_CONFIG;
	}

	INIReader meta_data(argv[2]);

	if (config.ParseError() < 0) {
		cerr << "Error parsing config file " << argv[2] << endl;
	while (1) {};
		return EX_CONFIG;
	}

	/*
	// Multi-threaded approach ************************************************

	// create new thread and run server
	log->info("Launching Killstreak server");
	server = new ServerGame(config);
	_beginthread(run_server, 0, 0);

	// launch client on main thread
	log->info("Launching Killstreak client");
	client = new ClientGame(config);
	client->run();

	// ************************************************************************
	*/





	// ---- Production version (client and server run on different machines)
//	/**************************************************************************
	// running server or client session (marked enabled in config)
	if (config.GetBoolean("server", "enabled", true))	
	{
		log->info("Launching Killstreak server");
		server = new ServerGame(config,meta_data);
		server->launch();
	}
	else if (config.GetBoolean("client", "enabled", true))
	{
		log->info("Launching Killstreak client");
		client = new ClientGame(config);
		client->run();
	} else {
		log->error("Neither Client/Server enabled (config)");
	}
//	***************************************************************************/




	return 0;
}
