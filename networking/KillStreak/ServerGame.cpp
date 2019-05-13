#include "sysexits.h"
#include "logger.hpp"
#include "ServerGame.hpp"
#include "ServerNetwork.hpp"
#include "nlohmann\json.hpp"
#include <fstream>

#include <string>
#include <process.h>				// threads
#include <windows.h>				// sleep

#define GAME_SIZE			1		// total players required to start game
#define LOBBY_START_TIME	2000	// wait this long (ms) after all players connect

static int game_start = 0;			// game ready to begin?
using json = nlohmann::json;

/*
	Parse data from config file for server. Then initialize 
	server network (create socket) and initialize data.
*/
ServerGame::ServerGame(string host, string port, double tick_rate) 
{
	auto log = logger();

	// get server data from config file
	/*
	string servconf = config.Get("server", "host", "");
	if (servconf == "") {
		log->error("Host line not found in config file");
		exit(EX_CONFIG);
	}

	size_t idx = servconf.find(":");		
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
	*/

	this->host = host.c_str();
	this->port = port.c_str();
	this->tick_rate = tick_rate;

	// initialize skills map
	vector<Skill> mage_skills;
	vector<Skill> assassin_skills;
	vector<Skill> warrior_skills;
	skill_map[ArcheType::MAGE] = mage_skills;
	skill_map[ArcheType::ASSASSIN] = assassin_skills;
	skill_map[ArcheType::WARRIOR] = warrior_skills;

	readMetaDataForSkills();

	scene = new ServerScene();


	network = new ServerNetwork(this->host, this->port);
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

			ServerInputPacket welcome_packet = createWelcomePacket();
			int bytes_sent = network->sendToClient(client_id - 1, welcome_packet);

			if (!bytes_sent) {	// error? 
				log->error("CT <{}>: Sending init packet to client failed, closing connection", client_id);
				network->closeClientSocket(client_id);
				return;
			}

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

	// LOBBY OVER -> CHARACTER SELECTION STARTING ***************************************************

	// all clients connected, wait LOBBY_START_TIME (ms) before starting character selection
	log->info("MT: Character selection starting in {} seconds...", LOBBY_START_TIME/1000);
	Sleep(LOBBY_START_TIME);					

	log->info("MT: Broadcasting character selection packet to all clients!");

	// broadcast character selection packet to all clients (tell them to select username/character)
	ServerInputPacket char_select_packet = createCharSelectPacket();
	network->broadcastSend(char_select_packet);


	// wait for character selection packet from each client (block on recv())
	for (auto client_data : client_data_list) {		
		int client_id = client_data->id;
		ClientSelectionPacket* selection_packet = network->receiveSelectionPacket(client_id);
		log->debug("received selection packet from a client");
		// TODO: Store this data somewhere on the server mapped to client
		std::string username = selection_packet->username;
		ArcheType character_type = selection_packet->type;
		PlayerMetadata player = PlayerMetadata(client_id, username, character_type);
		playerMetadatas.insert({ client_id, player });


		log->debug("Client {}: Username {}, Character: {}", client_id, username, character_type);
	}


	log->info("All client character selections received, starting game...");
	// TODO: broadcast game start to all clients once all characters selected
	//	--> ATTACH MODEL IDS TO SCENE GRAPH AND BROADCAST TO ALL CLIENTS in start_game packet from server
	//
	// TODO: Fix: Graphics this is where to send init scene graph!!
	// ....
	// ....
	// ....
	// ....


	for (auto client_data : client_data_list) {
		unsigned int client_id = client_data->id;
		scene->addPlayer(client_id, playerMetadatas[client_id].type);
	}

	// Init players in the scene
	for (auto client_data : client_data_list) {
		unsigned int client_id = client_data->id;
		char buf[1024] = { 0 };
		ServerInputPacket initScene_packet = createInitScenePacket(client_id, scene->scenePlayers[client_id].root_id);
		int bytes_sent = network->sendToClient(client_id, initScene_packet);
	}

	scene->initEnv();


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
	/*
	log->debug("MT: Sending test packets to client");
	char buf[1024] = "The first packet!";
	ServerInputPacket welcome_packet = network->createServerPacket(INIT_SCENE, 0, buf);
	int bytes_sent = network->sendToClient(0, welcome_packet);
	log->debug("MT: Type: {}", INIT_SCENE);
	log->debug("MT: Sending packet data: {}", bytes_sent);

	char buf2[1024] = "The second packet!";
	ServerInputPacket welcome_packet2 = network->createServerPacket(UPDATE_SCENE_GRAPH, 0, buf2);
	int bytes_sent2 = network->sendToClient(0, welcome_packet2);
	log->debug("MT: Type: {}", UPDATE_SCENE_GRAPH);
	log->debug("MT: Sending packet data: {}", buf2);

	char buf3[1024] = "The third packet!";
	ServerInputPacket welcome_packet3 = network->createServerPacket(INIT_SCENE, 0, buf3);
	int bytes_sent3 = network->sendToClient(0, welcome_packet3);
	log->debug("MT: Type: {}", INIT_SCENE);
	log->debug("MT: Sending packet data: {}", buf3);

	log->debug("MT: All three packets sent to client");
	*/

	//while (1) {};	// TODO: REMOVE ME!!!!




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
	// log->info("MT: Game server kill phase update...");

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
		for (auto &packet : inputPackets[i]) {
			handleClientInputPacket(packet, i);
		}
	}
	scene->update();

	// Serialize scene graph and send packet to clients
	/*char buf[1024] = { 0 };
	int size = scene->serializeSceneGraph(buf);
	ServerInputPacket sceneGraphPacket = network->createServerPacket(UPDATE_SCENE_GRAPH, size, buf); */
	ServerInputPacket serverTickPacket = createServerTickPacket();
	network->broadcastSend(serverTickPacket);

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

void ServerGame::readMetaDataForSkills() {
	Skill::load_archtype_data(skill_map); // update skill_map with all archtype meta_data

	/*
	skill_map[ArcheType::MAGE].push_back(Skill::getMelee(meta_data, "MAGE"));
	skill_map[ArcheType::MAGE].push_back(Skill::getProjectile(meta_data, "MAGE"));
	skill_map[ArcheType::MAGE].push_back(Skill::getAoe(meta_data, "MAGE"));
	skill_map[ArcheType::MAGE].push_back(Skill::getAoe(meta_data, "MAGE"));

	skill_map[ArcheType::ASSASSIN].push_back(Skill::getMelee(meta_data, "ASSASSIN"));
	skill_map[ArcheType::ASSASSIN].push_back(Skill::getProjectile(meta_data, "ASSASSIN"));
	skill_map[ArcheType::ASSASSIN].push_back(Skill::getAoe(meta_data, "ASSASSIN"));
	skill_map[ArcheType::ASSASSIN].push_back(Skill::getMinimap(meta_data, "ASSASSIN"));
	skill_map[ArcheType::ASSASSIN].push_back(Skill::getInvisible(meta_data, "ASSASSIN"));

	skill_map[ArcheType::WARRIOR].push_back(Skill::getMelee(meta_data, "WARRIOR"));
	skill_map[ArcheType::WARRIOR].push_back(Skill::getProjectile(meta_data, "WARRIOR"));
	skill_map[ArcheType::WARRIOR].push_back(Skill::getAoe(meta_data, "WARRIOR"));
	skill_map[ArcheType::WARRIOR].push_back(Skill::getCharge(meta_data, "WARRIOR"));
	*/
}


ServerInputPacket ServerGame::createInitScenePacket(unsigned int playerId, unsigned int playerRootId) {
	unsigned int sgSize;
	char buf[1024] = { 0 };
	char * bufPtr = buf;
	memcpy(bufPtr, &playerId, sizeof(unsigned int));
	bufPtr += sizeof(unsigned int);
	memcpy(bufPtr, &playerRootId, sizeof(unsigned int));
	bufPtr += sizeof(unsigned int);
	Transform * root = scene->getRoot();
	sgSize = Serialization::serializeSceneGraph(root, bufPtr, scene->serverSceneGraphMap);
	return createServerPacket(INIT_SCENE, 1024, buf);
}

ServerInputPacket ServerGame::createServerTickPacket() {
	unsigned int sgSize;
	char buf[SERVER_TICK_PACKET_SIZE] = { 0 };
	char * bufPtr = buf;
	sgSize = Serialization::serializeSceneGraph(scene->getRoot(), bufPtr, scene->serverSceneGraphMap);
	return createServerPacket(UPDATE_SCENE_GRAPH, SERVER_TICK_PACKET_SIZE, buf);
}


ServerInputPacket ServerGame::createServerPacket(ServerPacketType type, int size, char* data)
{
	ServerInputPacket packet;
	packet.packetType = type;
	packet.size = size;
	memcpy(packet.data, data, size);
	return packet;
}

ServerInputPacket ServerGame::createWelcomePacket() {
	char buf[1024] = { 0 };
	return createServerPacket(WELCOME, 0, buf);
}

ServerInputPacket ServerGame::createCharSelectPacket() {
	char buf[1024] = { 0 };
	return createServerPacket(CHAR_SELECT_PHASE, 0, buf);
}

void ServerGame::handleClientInputPacket(ClientInputPacket* packet, int client_id) {
	switch (packet->inputType) {
	case MOVEMENT:
		scene->handlePlayerMovement(client_id, packet->finalLocation);
		break;
	default:
		break;
	}
	free(packet);
}





