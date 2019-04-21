#include "sysexits.h"
#include "logger.hpp"
#include "ServerGame.hpp"
#include "ServerNetwork.hpp"

#include <string>
#include <process.h>				// threads
#include <windows.h>				// sleep

#define GAME_SIZE			1		// total players required to start game
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
	game starts. Will then consistently recv() data from client one packet size 
	at a time into a buffer. Once full packet is read, its deserialzied into a 
	ClientInputPacket struct and added to the client threads queue. Packets sent
	to the client threads queue are ready to be processed by the server.

	client_data passed to thread (arg), contains client ID & pointer to client
	thread queue & server network.

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
	while (!game_start) {};	 // TODO: Keep busy waiting or put threads to sleep?


	// GAME STARTING ****************************************************
	log->info("CT <{}>: Game started -> Receiving from client!", client_arg->id);

	// get client socket; pointer to clients queue & pointer to network
	int client_id = client_arg->id;
	ClientThreadQueue *input_queue = client_arg->q_ptr;
	ServerNetwork * network = client_arg->network;

	int bytes_read;						// total bytes read returned by recv()
	int keep_conn = 1;                  // keep connection alive
	do {

		// allocate buffer & receive data
		char temp_buff[sizeof(ClientInputPacket)];		
		memset(temp_buff, 0, sizeof(temp_buff));
		bytes_read = network->receiveData(client_id, temp_buff);
		
		log->info("CT {}: Total bytes read {} from ServerNetwork::receive", client_id, bytes_read);

		if (bytes_read == 0)			// connection closed?
		{
			log->info("CT {}: Client closed connection.", client_id);
			keep_conn = 0;
			break;
		}

		if (bytes_read == SOCKET_ERROR)  // error?  Close connection.
		{
			log->error("CT {}: recv() failed {}", client_id, WSAGetLastError());
			WSACleanup();
			keep_conn = 0;
			break;
		}

		// convert temp_buff into a ClientInputPacket (deserialize data)
		// TODO: might run into alignment issues later
		ClientInputPacket* packet = reinterpret_cast<ClientInputPacket*>(temp_buff);

		log->info("RECEIVED ON SERVER: ");
		log->info("packet input type: {}", packet->inputType);
		log->info("packet final location x: {}", (packet->finalLocation).x);
		log->info("y: {}", (packet->finalLocation).y);
		log->info("z: {}", (packet->finalLocation).z);
		log->info("packet skillType: {}", packet->skillType);
		log->info("packet attackType: {}", packet->attackType);

		input_queue->push(*packet);		// push full packet to client threads queue

	} while (keep_conn);				// connection-closed/error? 


	// close socket & free client_data 
	client_arg->network->closeClientSocket(client_arg->id);
	free(client_arg);
}


/*
	Handles the game lobby. Accepts incoming connections from clients until 
	enough players are found for one full game. Will wait LOBBY_START_TIME (ms)
	after all players connected to begin match by setting 'game_start' to 1.
*/
void ServerGame::game_match()
{
	auto log = logger();

	log->info("Size of overall inputpacketstruct: {}", sizeof(ClientInputPacket));
	log->info("Size of enum: {}", sizeof(InputType));
	log->info("Size of Point: {}", sizeof(Point));
	log->info("Size of int: {}", sizeof(int));

	// Accept incoming connections until GAME_SIZE met (LOBBY)
	unsigned int client_id = 0;		
	while (client_id < GAME_SIZE)
	{
		log->info("MT: Waiting for {} player(s)...", GAME_SIZE - client_id);
		network->acceptNewClient(client_id);		// blocking

		// allocate data for new client thread & run thread
		client_data* client_arg = (client_data *) malloc (sizeof(client_data));
		ClientThreadQueue* client_q = new ClientThreadQueue();		// queue for client's packets
		clientThreadQueues.push_back(client_q);						// add to servers vector of all client queues

		if (client_arg)
		{
			client_arg->id = client_id - 1;				// current clients ID
			client_arg->q_ptr = client_q;				// pointer to clients packet queue
			client_arg->network = network;				// pointer to ServerNetwork
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

	// launch lobby; accept players until game full
	log->info("MT: Game server live - Launching lobby!");
	game_match();	

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
			log->info("TICK");
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

	// TODO: Get one packet from each client queue. Maintain round robin order switching order 
	// of getting packets.
}





