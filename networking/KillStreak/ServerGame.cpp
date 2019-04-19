#include "sysexits.h"
#include <string>
#include "ServerGame.hpp"
#include "ServerNetwork.hpp"
#include "logger.hpp"

#define GAME_SIZE	1	// total players required to start game


ServerGame::ServerGame(INIReader& t_config) : config(t_config) {
	auto log = logger();

	// get server data from config file
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

	network = new ServerNetwork(host, port);
}


/* 
	Main server loop run once the network is setup. Server will block on accept until 
	enough players have joined then begin the main game loop. 
*/
void ServerGame::launch() {
	auto log = logger();
	log->info("Game server live!");

	/* 
		NOTE: Currently only waiting for 1 connection, then testing if we 
		recv() data sent by client.
	*/


	// Accept incoming connections until GAME_SIZE met
	unsigned int client_id = 0;		// TODO: Make this static in this script?	
	while (client_id < GAME_SIZE)
	{
		log->info("Waiting for {} player(s)...", GAME_SIZE - client_id);
		network->acceptNewClient(client_id);
	}


	// TESTING: Is server receiving clients data?
	int iResult;
	SOCKET client_sock = network->sessions.find(0)->second;	// get client socket 
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	do {

		iResult = recv(client_sock, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			log->info("Bytes received: {}", iResult);
		}
		else if (iResult == 0)
		{
			log->info("Connection closed");
			printf("Connection closed\n");
		}
		else
		{
			log->info("recv failed: {}", WSAGetLastError());
		}

	} while (iResult > 0);


	while (1) {}; // TODO: REMOVE ME!!


	/* TODO: Stop listening to socket once all connections made? 
		 --> Or we could keep listening, clients would queue up and if one drop sout 
		     we can replace them with the next one in the queue
	*/


	log->info("All players connected, game starting!");


	/* TODO: Send pre-game data to all clients? 
		--> Or send when connection is accepted
		--> Then once all players connected we can immedietly start? 
	*/


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
	// TODO: Will server iterate over all clients and recv() updates? 
	// --> update game state?
	// --> send() to all clients?
}



