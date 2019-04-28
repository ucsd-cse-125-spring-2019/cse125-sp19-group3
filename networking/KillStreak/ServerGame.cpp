#include "sysexits.h"
#include "logger.hpp"
#include "ServerGame.hpp"
#include "ServerNetwork.hpp"

#include <string>
#include <process.h>				// threads
#include <windows.h>				// sleep

#define GAME_SIZE			1		// total players required to start game
#define LOBBY_START_TIME	2000	// wait this long (ms) after all players connect

static int game_start = 0;			// game ready to begin?


/*
	Parse data from config file for server. Then initialize 
	server network (create socket) and initialize data.
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

	scene = new ServerScene();
	scene->initialize_objects();

	network = new ServerNetwork(host, port);
	scheduledEvent = ScheduledEvent(END_KILLPHASE, 10000000); // default huge value

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
	int client_id = client_arg->id;
	ServerNetwork * network = client_arg->network;

	log->info("CT <{}>: Launching new client thread", client_id);

	// send pre-game data to client telling them they're accepted ( meta data and lobby info )
	char buf[1024] = { 0 };
	ServerInputPacket welcome_packet = network->createServerPacket(INIT_SCENE, 0, buf);
	int bytes_sent = network->sendToClient(client_id, welcome_packet);

	if (!bytes_sent) {	// error? 
		log->error("CT <{}>: Sending init packet to client failed, closing connection", client_id);
		network->closeClientSocket(client_id);
		return;
	}

	// game hasn't started yet; wait until ready
	if (!game_start) log->info("CT <{}>: Waiting for game to start", client_arg->id);
	while (!game_start) {};	 


	// GAME STARTING ****************************************************
	// receive from client until end of game sending deserialized packets to queue

	log->info("CT <{}>: Game started -> Receiving from client!", client_arg->id);

	// get client socket; pointer to clients queue & pointer to network
	ClientThreadQueue *input_queue = client_arg->q_ptr;

	int keep_conn = 1;					// keep connection alive
	do {

		// get packet from client
		ClientInputPacket* packet = network->receivePacket(client_id);
		if (!packet) {
			keep_conn = 0;
			break;
		}

		// acquire queue lock & push packet 
		client_arg->q_lock->lock();
		input_queue->push(packet);		
		client_arg->q_lock->unlock();

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

	// Accept incoming connections until GAME_SIZE met (LOBBY)
	unsigned int client_id = 0;		
	while (client_id < GAME_SIZE)
	{

		log->info("MT: Waiting for {} player(s)...", GAME_SIZE - client_id);

		// accpet inc. connection (blocking)
		bool ret_accept = network->acceptNewClient(client_id);		
		if (!ret_accept) continue; // failed to make conn?

		// allocate data for new client thread & run thread
		client_data* client_arg = (client_data *) malloc (sizeof(client_data));
		ClientThreadQueue* client_q = new ClientThreadQueue();		// queue for client's packets
		mutex* client_lock = new mutex();							// lock for clients queue

		if (client_arg)
		{
			client_arg->id = client_id - 1;				// current clients ID
			client_arg->q_ptr = client_q;				// pointer to clients packet queue
			client_arg->network = network;				// pointer to ServerNetwork
			client_arg->q_lock = client_lock;			// pointer to queue lock

			client_data_list.push_back(client_arg);		// add pointer to this clients data
			_beginthread(client_session, 0, (void*) client_arg);
		}
		else	// error allocating client data; decrement client_id, close socket, deallocate
		{
			log->error("MT: Error allocating client metadata");
			network->closeClientSocket(--client_id);	
			delete (client_q);
			delete (client_lock);
			free(client_arg);
		}

	}

	// all clients connected, wait LOBBY_START_TIME (ms) before starting character selection
	log->info("MT: Character selection starting in {} seconds...", LOBBY_START_TIME/1000);
	Sleep(LOBBY_START_TIME);					
	log->info("MT: Character selection started!");

	/*
	1) send startgame packet to all clients
		--> Hey client game is starting, exit the lobby, enter username and select character
		--> Tell them how much time they have to make selection
		*** NOTE: Need to modify client thread to expect this!
	2) start timer on server
	3) wait until you receive all finish start game init packets from clients && timer is up
		** Once timer is up send ack to clients telling them its starting with whatever 
			data is needed
	4) schedule end of kill phase
	*/

	// schedule end of kill phase
	ScheduledEvent initKillPhase(END_KILLPHASE, 60); // play with values in config file

	// Tell all client threads to begin the game!
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


	// TESTING: Sending multiple different packets to client testing their queue
	log->debug("MT: Sending test packets to client");
	char buf[1024] = { 0 };
	ServerInputPacket welcome_packet = network->createServerPacket(INIT_SCENE, 0, buf);
	int bytes_sent = network->sendToClient(0, welcome_packet);
	log->debug("MT: Sending packet 1 size: {}", bytes_sent);
	log->debug("MT: Type: {}", INIT_SCENE);

	char buf2[1024] = { 0 };
	ServerInputPacket welcome_packet2 = network->createServerPacket(UPDATE_SCENE_GRAPH, 0, buf2);
	int bytes_sent2 = network->sendToClient(0, welcome_packet2);
	log->debug("MT: Sending packet 2 size: {}", bytes_sent2);
	log->debug("MT: Type: {}", UPDATE_SCENE_GRAPH);

	char buf3[1024] = { 0 };
	ServerInputPacket welcome_packet3 = network->createServerPacket(INIT_SCENE, 0, buf3);
	int bytes_sent3 = network->sendToClient(0, welcome_packet3);
	log->debug("MT: Sending packet 3 size: {}", bytes_sent3);
	log->debug("MT: Type: {}", INIT_SCENE);

	log->debug("MT: All three packets sent to client");
	while (1) {};	// TODO: REMOVE ME!!!!




	// GAME START *************************************************

	double ns = 1000000000.0 / tick_rate;
	double delta = 0;
	bool running = true; // not sure if needed;
	auto lastTime = Clock::now();
	
	bool isKillPhase = true;

	log->info("Server is about to enter game loop!");
	// GAME LOOP
	while (running) {
		auto now = Clock::now();
		dsec ds = now - lastTime;
		nanoseconds duration = chrono::duration_cast<nanoseconds>(ds);
		delta += (duration.count() / ns);
		lastTime = now;

		while (delta >= 1) {
			// put kill phase vs prepare phase here?
			delta--;

			scheduledEvent.ticksLeft--;

			if (scheduledEvent.ticksLeft <= 0) {
				// initNewPhase(isKillPhase);
				isKillPhase = !isKillPhase;
				// pop off / get rid of event alarm here?
			}

			if (isKillPhase) {
				updateKillPhase();
			}
			else {
				updatePreparePhase();
			}
		}
	}
}


/*
	Runs on every server tick. Empties all client_thread queues, updates the game state, and 
	broadcasts updated state back to all clients.
*/
void ServerGame::updateKillPhase() {
	auto log = logger();
	log->info("MT: Game server kill phase update...");

	// create temp vectors for each client to dump all incoming packets into
	vector<vector<ClientInputPacket*>> inputPackets;
	for (int i = 0; i < GAME_SIZE; i++) {
		inputPackets.push_back(vector<ClientInputPacket*>());
	}

	// TODO: BE SURE TO FREE PACKETS AFTER PROCESSING THEM, OTHERWISE THE PACKETS WILL EVENTUALLY
	// FILL MEMORY!
	// Drain all packets from all client inputs
	for (auto client_data : client_data_list) {
		int i = client_data->id;
		ClientThreadQueue * q_ptr = client_data->q_ptr;

		// acquire lock; empty entire queue
		client_data->q_lock->lock();
		while (!(q_ptr->empty())) {
			inputPackets[i].push_back(q_ptr->front());
			q_ptr->pop();
		}
		client_data->q_lock->unlock();
	}

	// Handle packets
	for (int i = 0; i < GAME_SIZE; i++) {
		for (auto packet : inputPackets[i]) {

			log->info("Server received packet with input type {}, finalLocation of {}, {}, {}", 
				packet->inputType, packet->finalLocation.x, packet->finalLocation.y, packet->finalLocation.z);

			switch (packet->inputType) {
			MOVEMENT:
				scene->handlePlayerMovement(packet->finalLocation);
			}
		}
	}

	// Serialize scene graph and send packet to clients
	char buf[1024] = { 0 };
	int size = scene->serializeSceneGraph(buf);
	ServerInputPacket sceneGraphPacket = network->createServerPacket(UPDATE_SCENE_GRAPH, size, buf);

	network->broadcastSend(sceneGraphPacket);

	/*
	
	1) Drain all packets from all client inputs at this point (acquire lock), squash if necessary

	2) Apply input to change game state

	3) Use updated game state (movement, fired skill, etc.) to change server SceneGraph slightly

	4) Hit detection on all objects
		a) For each player, put in quant tree to see if hit skill / environment
		b) For each skill, put in quant tree to see if hit environment
	
	5) Do any calculations necessary for deaths (update leaderboard, update gold rewarded, update bonuses, etc)

	6) Serialize server SceneGraph, send leaderboard / gold/ time remaining in round / player state to all clients 

	7.) Deallocate packets 
	*/


}

void ServerGame::updatePreparePhase() {
	auto log = logger();
	log->info("MT: Game server update prepare phase...");
}





