#include "sysexits.h"
#include "logger.hpp"
#include "ServerGame.hpp"
#include "ServerNetwork.hpp"

#include <string>
#include <process.h>				// threads
#include <windows.h>				// sleep

#define GAME_SIZE			1		// total players required to start game
#define LOBBY_START_TIME	5000	// wait this long (ms) after all players connect
#define CLIENT_PACKET_SIZE	10000	// expected size in bytes of packet sent from client->server

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

	// TODO: recv() data from each client and ...
	//	--> DISCUSS: Do we want to assume one full packet will be sent at a time? 
	//			** Or would we rather take everything, look for delimiter and then send packet to queue (slower?)
	//	--> How large do we make the buffer?!?!?! How big are our packets going to be?!?!
	//	--> Serialize or just send byte stream?!?!?!
	//  --> Put into main buffer until delimiter reached when calling receive 
	//		*** NEED TO PUT DELIMITER TO DENOTE END OF DATA!!!
	//  --> Once all data received push to master queue 
	//  --> Master queue will update state by getting all data from the queue 
	//			in FIFO order. Then send updates back to the client.

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



	// GAME STARTING ****************************************************
	log->info("CT <{}>: Game started -> Receiving from client!", client_arg->id);

	// get client socket
	int client_id = client_arg->id;
	ClientThreadQueue *input_queue = client_arg->q_ptr;
	ServerNetwork * network = client_arg->network;

	// vector<char> main_buffer;			// all bytes received so far
	int bytes_read;						// total bytes read returned by recv()
	int keep_conn = 1;                  // keep connection alive
	// int last_index = -1;				// last index of complete request in buffer (end of packet)
	do {

		// allocate buffer & receive data;
		/* TODO: Play around with CLIENT_PACKET_SIZE
			--> Make it the largest size we expect to receive a packet from the client
		*/
		char temp_buff[sizeof(ClientInputPacket)];		
		memset(temp_buff, 0, sizeof(temp_buff));

		bytes_read = network->receiveData(client_id, temp_buff);
		
		// bytes_read = recv(client_sock, temp_buff, CLIENT_PACKET_SIZE, 0);
		log->info("CT {}: Total bytes read {} from ServerNetwork::receive", client_id, bytes_read);

		if (bytes_read == 0)    // connection closed?
		{
			log->info("CT {}: Client closed connection.", client_arg->id);
			keep_conn = 0;
			break;
		}

		if (bytes_read == SOCKET_ERROR)     // error?  Close connection.
		{
			log->error("CT {}: recv() failed {}", client_id, WSAGetLastError());
			WSACleanup();
			keep_conn = 0;
			break;
		}

		// convert temp_buff into a ClientInputPacket; might run into alignment issues later
		ClientInputPacket* packet = reinterpret_cast<ClientInputPacket*>(temp_buff);

		log->info("RECEIVED ON SERVER: PLS WORK!!");
		log->info("packet input type: {}", packet->inputType);
		log->info("packet final location x: {}", (packet->finalLocation).x);
		log->info("y: {}", (packet->finalLocation).y);
		log->info("z: {}", (packet->finalLocation).z);
		log->info("packet skillType: {}", packet->skillType);
		log->info("packet attackType: {}", packet->attackType);


		input_queue->push(*packet);
		


	} while (keep_conn);	// connection-closed/error? 


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

		ClientThreadQueue* client_q = new ClientThreadQueue();
		clientThreadQueues.push_back(client_q);


		if (client_arg)
		{
			client_arg->id = client_id - 1;			// current clients ID
			client_arg->q_ptr = client_q;				// pointer to master queue
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


	game_match();	// launch lobby; accept players until game full


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





