#include "sysexits.h"
#include "logger.hpp"
#include "ServerGame.hpp"
#include "ServerNetwork.hpp"

#include <string>
#include <process.h>				// threads
#include <windows.h>				// sleep

#define GAME_SIZE			2		// total players required to start game
#define LOBBY_START_TIME	5000	// wait this long (ms) after all players connect

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
	game starts. Will then consistently recv() data from client storing 
	into a buffer until the delimiter is reached. Once reached the data is 
	sent to the master queue to be processed by the server.

	client_data passed to thread (arg), contains client ID & pointer to master queue.
	Note: log->info( ... ) 'CT {}:' stands for 'Client Thread <ID>:' 
*/
void client_session(void *arg)
{
	auto log = logger();

	// get client data
	client_data *client_arg = (client_data*) arg;
	log->info("CT <{}>: Launching new client thread", client_arg->id);

	// game hasn't started yet; sleep thread until ready
	if (!game_start) log->info("CT <{}>: Waiting for game to start", client_arg->id);

	// TODO...
	// BADD!!!!! BUSY WAITING!!!! Replace by putting thread to sleep until ready!
	// NOTE: Need this thread to sleep until the main thread decides its time to 
	//		start the game. Once started the main thread will wake all sleeping threads.
	while (!game_start) {};	

	log->info("CT <{}>: Game started -> Receiving from client!", client_arg->id);

	// TODO: recv() data from each client and ...
	//	--> How large do we make the buffer?!?!?! How big are our packets going to be?!?!
	//	--> Serialize or just send byte stream?!?!?!
	//  --> Put into main buffer until delimiter reached when calling receive 
	//		*** NEED TO PUT DELIMITER TO DENOTE END OF DATA!!!
	//  --> Once all data received push to master queue 
	//  --> Master queue will update state by getting all data from the queue 
	//			in FIFO order. Then send updates back to the client.

	// TESTING: Server receiving data from client and sending response?
	int iResult;
	SOCKET client_sock = client_arg->network->sessions.find(client_arg->id)->second;	// get client socket 
	char recvbuf[DEFAULT_BUFLEN];	// default 512 bytes
	int recvbuflen = DEFAULT_BUFLEN;
	do {

		iResult = recv(client_sock, recvbuf, recvbuflen, 0);
		if (iResult > 0)	// success
		{
			recvbuf[iResult] = '\0'; // NULL terminate buffer
			log->info("CT {}: Bytes received: {}", client_arg->id, iResult);
			log->info("CT {}: Data received: {}", client_arg->id, recvbuf);

			// send response to client
			char* sendbuf = "Smile if you're seeing this, because it works :D!!!!";
			int iResult = send(client_sock, sendbuf, (int)strlen(sendbuf), 0);
			if (iResult == SOCKET_ERROR) {
				wprintf(L"send failed with error: %d\n", WSAGetLastError());
				WSACleanup();
				return;
			}
			log->info("CT {}: Sending response to client!", client_arg->id);
			
		}
		else if (iResult == 0)	// client closed connection
		{
			log->info("CT {}: Connection closed", client_arg->id);
			log->info("CT {}: Connection closed\n", client_arg->id);
		}
		else					// error
		{
			log->info("CT {}: recv failed: {}", client_arg->id, WSAGetLastError());
		}

	} while (iResult > 0);  // ensures loop continues until client closes connection/error occurs 


	while (1) {};	// TODO: REMOVE ME!!!

	// close socket & free client_data 
	client_arg->network->closeClientSocket(client_arg->id);
	free(client_arg);
}


/*
	Handles the game lobby. Accepts incoming connections from clients until 
	enough players are found for one full game. Will wait LOBBY_START_TIME (ms)
	after all players connected to begin match by setting 'game_start' to 1.
*/
void ServerGame::game_match(MasterQueue *mq)
{
	auto log = logger();

	// Accept incoming connections until GAME_SIZE met (LOBBY)
	unsigned int client_id = 0;		
	while (client_id < GAME_SIZE)
	{
		log->info("MT: Waiting for {} player(s)...", GAME_SIZE - client_id);
		network->acceptNewClient(client_id);		// blocking

		// allocate data for new client thread & run thread
		client_data* client_arg = (client_data *) malloc (sizeof(client_data));
		if (client_arg)
		{
			client_arg->id = client_id - 1;			// current clients ID
			client_arg->mq_ptr = mq;				// pointer to master queue
			client_arg->network = network;			// pointer to ServerNetwork
			_beginthread(client_session, 0, (void*) client_arg);
		}
		else	// error allocating client data; decrement client_id & remove socket
		{
			log->error("MT: Error allocating client metadata");
			network->closeClientSocket(--client_id);	
		}
	}

	// all clients connected, wait LOBBY_START_TIME (ms) before starting game
	log->info("MT: Game starting in {} seconds...", LOBBY_START_TIME/1000);
	Sleep(LOBBY_START_TIME);			
	log->info("MT: Game started!");

	// TODO: Wake all sleeping client threads once busy waiting is removed.
	game_start = 1;			

}


/* 
	Main server loop that runs after the network is setup. 
	Server will block on game_match until enough players have joined,
	then it'll begin the main game loop. 
	A new thread is created for each incoming client.

	Note: log->info( ... ) 'MT' stands for 'Main Thread'
*/
void ServerGame::launch() {
	auto log = logger();
	log->info("MT: Game server live!");

	// TODO: protect master_queue w/ CV; pass pointer to client thread
	MasterQueue* mq = new MasterQueue();

	game_match(mq);	// launch lobby; accept players until game full

	while (1) {};	// TODO: REMOVE ME!!!


	/* TODO: Send pre-game data to all clients? 
		--> Or send when connection is accepted
		--> Then once all players connected we can immedietly start? 
	*/

	/* 
		IDEA: Client threads constantly getting input from user and immedietly putting 
		into global queue protected by CV. Server loop will clear the queue on 
		every tick updating the game state and telling all clients?
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


/*
	Runs on every server tick. Empties the master queue, updates the game state, and 
	sends updated state back to all clients.
*/
void ServerGame::update() {
	auto log = logger();
	log->info("MT: Game server update...");

	// TODO: Get all packets from queue and update then send back to clients


}





