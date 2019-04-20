#include "sysexits.h"
#include "logger.hpp"
#include "ServerGame.hpp"
#include "ServerNetwork.hpp"
#include <string>
#include <process.h>				// threads
#include <windows.h>				// sleep

#define GAME_SIZE			1		// total players required to start game
#define LOBBY_START_TIME	5		// wait for this long after all players connect

static int game_start = 0;			// game ready to begin?


/*
	Parse data from config file for server. Then initialize 
	server network (create socket).
*/
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
	Launch thread for new client. Waits until all players connected and 
	game starts. Will then consistently recv() data from client and 
	input full packets into master queue. 
*/
void client_session(void *arg)
{
	auto log = logger();

	// save client ID
	const unsigned int cur_id = (unsigned int)arg;	
	log->info("Launching new client thread; Client_ID: {}", cur_id);

	// game hasn't started yet; sleep thread until ready
	if (!game_start) log->info("{}: Waiting for game to start", cur_id);

	while (!game_start) {};	// TODO: Remove me!!! Put thread to sleep, no busy waiting!

	log->info("{}: Game started -> Receiving from client!", cur_id);
	while (1) {};	// TODO: REMOVE ME!!! -> Start receiving data

}


/* 
	Main server loop run once the network is setup. Server will block on accept until 
	enough players have joined then begin the main game loop. 
*/
void ServerGame::launch() {
	auto log = logger();
	log->info("Game server live!");

	// Accept incoming connections until GAME_SIZE met
	unsigned int client_id = 0;		
	while (client_id < GAME_SIZE)
	{
		log->info("Waiting for {} player(s)...", GAME_SIZE - client_id);
		network->acceptNewClient(client_id);
	
		// allocate new thread for client (pass ID)
		_beginthread(client_session, 0, (void*) (client_id-1));

		// --> First successfully get string and print it? 
		// launch new client session thread whose only job is to recv() data 
		// in put it into a queue? 
		// --> Test by having 2 clients send data then printing everything in queue?
	}


	// all clients connected, wait LOBBY_START_TIME seconds before starting game
	log->info("Game starting in {} seconds...", LOBBY_START_TIME);
	Sleep(LOBBY_START_TIME);			
	log->info("Game started!");
	game_start = 1;			

	while (1) {};	// TODO: REMOVE ME!!!


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
			log->info("Data received: {}", recvbuf);
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
		 --> Or we could keep listening, clients wo// start gameuld queue up and if one drop sout 
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

	/* 
		IDEA: Client threads constantly getting input from user and immedietly putting 
		into global queue protected by CV. Server loop will clear the queue on 
		every tick updating the game state and telling all clients?
	*/
	
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



