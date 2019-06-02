#include "sysexits.h"
#include "logger.hpp"
#include "ServerGame.hpp"
#include "ServerNetwork.hpp"
#include "PlayerData.hpp"
#include "CoreTypes.hpp"
#include "nlohmann\json.hpp"
#include <fstream>

#include <string>
#include <process.h>					// threads
#include <windows.h>					// sleep

#define LOBBY_START_TIME	 500		// wait this long (ms) after all players connect
#define START_CHAR_SELECTION 1			// start value of char selection phase
#define END_CHAR_SELECTION   2			// end value of char selection phase

static int game_start			  = 0;	// game ready to begin?
static int character_select_start = 0;	// character selection ready to begin?
static int character_select_over  = 0;	// character selection over?

using json = nlohmann::json;
using namespace std;


/*
	Parse data from config file for server. Then initialize 
	server network (create socket) and initialize data.
*/
ServerGame::ServerGame(string host, string port, double tick_rate) 
{
	auto log = logger();

	this->host = host.c_str();
	this->port = port.c_str();
	this->tick_rate = tick_rate;

	// initialize maps
	skill_map		    = new unordered_map<unsigned int, Skill>();
	selected_characters = new unordered_map<ArcheType, int>();
	playerMetadatas     = new unordered_map<unsigned int, PlayerMetadata*>();
	archetype_skillset  = new unordered_map<ArcheType, vector<unsigned int>>();

	archetypes = vector<int>(GAME_SIZE);
	usernames = vector<string>(GAME_SIZE);

	// initialize global skill map; load skills from config into skills maps
	readMetaDataForSkills();	

	leaderBoard = new LeaderBoard();
	scene		= new ServerScene(leaderBoard, playerMetadatas, skill_map, archetype_skillset);
	network		= new ServerNetwork(this->host, this->port);
	scheduledEvent = ScheduledEvent(END_KILLPHASE, 10000000); // default huge value

	char_select_lock = new mutex();

}


/*
	Receives clients packet to request a character and username. Checks if character 
	has already been selected, if so sends packet to user informing them of unavailable 
	characters. Will continue waiting for client input until the client selects a valid character.
*/
void character_selection_phase(client_data* client_arg)
{

	logger()->info("CT <{}>: Waiting for clients character selection", client_arg->id);

	int selected = 0;
	int client_id = client_arg->id;
	ServerNetwork * network = client_arg->network;

	do
	{

		// block on recv() until get clients character selection
		ClientSelectionPacket* selection_packet = network->receiveSelectionPacket(client_id);
		ArcheType character_type = selection_packet->type;

		// grab character select lock & check if character available
		client_arg->char_lock->lock();
		unordered_map<ArcheType, int>::iterator c_it = client_arg->selected_chars_map_ptr->find(character_type);

		// unavailable.. tell client what characters have been selected; loop again;
		if (c_it != client_arg->selected_chars_map_ptr->end())	
		{
			logger()->info("Client <{}>: Requested character unavailable", client_id);
			
			string data_str = "0";				// first byte 0 means failure

			// find all unavailable characters & add to enum index to data
			c_it = client_arg->selected_chars_map_ptr->begin();
			while ( c_it != client_arg->selected_chars_map_ptr->end())
			{
				data_str += to_string(c_it->first);
				c_it++;
			}

			client_arg->char_lock->unlock();	// release lock

			// convert to char* 
			int sz = data_str.length();
			char data[1024];
			strcpy(data, data_str.c_str());
			logger()->info("Client <{}>: Sending request to pick again!", client_id);

			// send packet
			ServerInputPacket char_response_packet = network->createCharSelectPacket(data, sz);
			network->sendToClient(client_id, char_response_packet);

		}
		else	// character available
		{

			// make character unavailable
			client_arg->selected_chars_map_ptr->insert({ character_type, client_id });

			// allocate meta data 
			std::string username = selection_packet->username;
			PlayerMetadata* player = new PlayerMetadata(client_id, username, character_type, 
				client_arg->skill_map_ptr, client_arg->archetype_skillset_ptr);

			// add to map & release lock
			client_arg->playerMetadatas_ptr->insert({ client_id, player });
			client_arg->char_lock->unlock();		 

			// send packet telling client of successful selection (1 as first byte of data)
			char data[] = "1";		
			ServerInputPacket char_response_packet = network->createCharSelectPacket(data,1);
			network->sendToClient(client_id, char_response_packet);

			logger()->info("Client <{}>: Selected Username {} and ArcheType: {}", client_id, username, character_type);
			selected = 1;		// end loop
		}

	} while (!selected);		// until unique character selected

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

	// block until character selection phase
	while (character_select_start != START_CHAR_SELECTION);

	// handle clients character selection
	character_selection_phase(client_arg);

	// end character selection phase if all characters selected
	if (client_arg->selected_chars_map_ptr->size() == GAME_SIZE) character_select_over = END_CHAR_SELECTION;

	// busy wait if game hasn't started yet...
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
	Accept incoming connections up to GAME_SIZE. Launch a new thread 
	for each accepted client. Each thread will recv() indefinently add 
	packets to the main queue.
*/
void ServerGame::launch_client_threads()
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

			// pointers to servers maps
			client_arg->skill_map_ptr = skill_map;
			client_arg->playerMetadatas_ptr = playerMetadatas;
			client_arg->archetype_skillset_ptr = archetype_skillset;
			client_arg->selected_chars_map_ptr = selected_characters;

			// pointers to servers locks
			client_arg->q_lock = client_lock;			
			client_arg->char_lock = char_select_lock;

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

}


/*
	Handles the game lobby. Accepts incoming connections from clients until 
	enough players are found for one full game. Will wait LOBBY_START_TIME (ms)
	after all players connected to begin match by setting 'game_start' to 1.
*/
void ServerGame::game_match()
{
	auto log = logger();

	// accept incoming connections & launch new thread for each client
	launch_client_threads();

	// all clients connected, wait LOBBY_START_TIME (ms) before starting character selection
	log->info("MT: Character selection starting in {} seconds...", LOBBY_START_TIME/1000);
	Sleep(LOBBY_START_TIME);					

	// broadcast character selection packet to all clients (tell them to select username/character)
	log->info("MT: Broadcasting character selection packet to all clients!");
	char buf[1024] = { 0 };
	ServerInputPacket char_select_packet = createCharSelectPacket(buf,0);
	network->broadcastSend(char_select_packet);

	// start character selection & wait for it to complete (handled by client threads)
	character_select_start = START_CHAR_SELECTION;		
	logger()->info("MT: Waiting for character selection phase to end");
	while (character_select_over != END_CHAR_SELECTION);

	// add each player to scene based on character selection
	log->info("All client character selections received, initializing game...");
	for (auto client_data : client_data_list) {
		unsigned int client_id = client_data->id;
		unordered_map<unsigned int, PlayerMetadata*>::iterator m_it = playerMetadatas->find(client_id);
		ArcheType cur_type	  = m_it->second->type;
		usernames[client_id]  = m_it->second->username;		// set username
		archetypes[client_id] = cur_type;					// set archetype
		scene->addPlayer(client_id, cur_type);
	}

	// Init players in the scene; send initScene packet to all clients (tells players game is initializing)
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

	// Process packets from client (movement, attack, etc..)
	for (int i = 0; i < GAME_SIZE; i++) {
		for (auto &packet : inputPackets[i]) {
			handleClientInputPacket(packet, i);
		}
	}
	scene->update();	// update scene graph

	// Serialize scene graph & leaderboard -> send packet to clients
	ServerInputPacket serverTickPacket = createServerTickPacket();

	// send packet to each client; update packet if they died this tick
	unordered_map<unsigned int, PlayerMetadata*>::iterator p_it = playerMetadatas->begin();
	while (p_it != playerMetadatas->end())
	{
		unsigned int client_id = p_it->first;
		PlayerMetadata* player_meta = p_it->second;
		
		// make copy of server tick packet
		ServerInputPacket next_packet; 
		memcpy(&next_packet, &serverTickPacket, sizeof(serverTickPacket));

		// set first byte of data to players dead/alive state
		memcpy(next_packet.data, &p_it->second->alive, sizeof(bool));

		network->sendToClient(client_id, next_packet);
		p_it++;
	}

}

void ServerGame::updatePreparePhase() {
	auto log = logger();
	log->info("MT: Game server update prepare phase...");
}


// update skill_map with all archtype meta_data
void ServerGame::readMetaDataForSkills() {
	Skill::load_archtype_data(skill_map, archetype_skillset);
}


ServerInputPacket ServerGame::createInitScenePacket(unsigned int playerId, unsigned int playerRootId) {
	// init scene packet will have the first scenegraph
	unsigned int sgSize;
	char buf[10000] = { 0 };
	char * bufPtr = buf;

	// serialize all usernames; PlayerMeta.username
	for (int client_id = 0; client_id < usernames.size(); client_id++)
	{
		char truncUsername[16] = { 0 };
		memcpy(truncUsername, usernames[client_id].c_str(), usernames[client_id].length());

		// serialize sizusername 
		memcpy(bufPtr, truncUsername, 16);
		bufPtr += 16;
	}

	// serialize all archetypes
	for (int client_id = 0; client_id < archetypes.size(); client_id++)
	{
		memcpy(bufPtr, &archetypes[client_id], sizeof(int));
		bufPtr += sizeof(int);
	}

	memcpy(bufPtr, &playerId, sizeof(unsigned int));
	bufPtr += sizeof(unsigned int);
	memcpy(bufPtr, &playerRootId, sizeof(unsigned int));
	bufPtr += sizeof(unsigned int);
	Transform * root = scene->getRoot();
	sgSize = Serialization::serializeSceneGraph(root, bufPtr, scene->serverSceneGraphMap);
	return createServerPacket(INIT_SCENE, 10000, buf);
}

//static int packetCounter = 0;

/*
	Create packet with serialized scene graph and leaderboard.
*/
ServerInputPacket ServerGame::createServerTickPacket() {
	ServerInputPacket packet;		
	unsigned int sgSize = 0;
	char buf[SERVER_TICK_PACKET_SIZE] = { 0 };

	char* headPtr = buf; // point to start of buffer
	char* bufPtr = buf;	

	// serialize if user died; default init the first byte (died_this_tick)
	bool died_this_tick = false;
	memcpy(bufPtr, &died_this_tick, sizeof(died_this_tick));
	sgSize += sizeof(died_this_tick);
	bufPtr += sizeof(died_this_tick);

	memcpy(bufPtr, &(scene->warriorIsCharging), sizeof(bool));
	bufPtr += sizeof(bool);
	sgSize += sizeof(bool);
	//scene->warriorIsCharging = false;

	unsigned int animation_size = 0;
	animation_size = Serialization::serializeAnimationMode(scene->scenePlayers, bufPtr); // TODO: double check that this function is correctly returning size
	bufPtr += animation_size;
	sgSize += animation_size;

	// serealize leaderboard
	unsigned int leaderBoard_size = 0;
	leaderBoard_size = Serialization::serializeLeaderBoard(bufPtr, leaderBoard);
	bufPtr += leaderBoard_size;
	sgSize += leaderBoard_size;

	unsigned int sceneGraph_size = 0;
	sceneGraph_size += Serialization::serializeSceneGraph(scene->getRoot(), bufPtr, scene->serverSceneGraphMap);
	sgSize += sceneGraph_size;

	// copy all serialized data into packet.data 1 byte offset for boolean (died_this_tick)
	packet.packetType = UPDATE_SCENE_GRAPH;
	packet.size = sgSize;
	memcpy(packet.data, headPtr, sgSize); 

	return packet;

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

/*
	Create packet telling clients to select their username and character.
	Result of -1 is initial packet with no body, otherwise serialzie data. 
	First integer is result (0 for failure, 1 for success).
	Second integer is the concatenated indices that are unavailable in the 
	character enum types. 
		--> Example: If HUMAN and MAGE are unavailable then 01 will be the value
		--> Can then read one byte at a time
*/
ServerInputPacket ServerGame::createCharSelectPacket(char* data, int size)
{
	return createServerPacket(CHAR_SELECT_PHASE, size, data);
}

/*
	Process client input (movement, attack, etc..). 
	Delegates to function corresponding to packet-type. 
*/
void ServerGame::handleClientInputPacket(ClientInputPacket* packet, int client_id) {

	unordered_map<unsigned int, PlayerMetadata*>::iterator m_it = playerMetadatas->find(client_id);
	PlayerMetadata* player_metadata = m_it->second;

	switch (packet->inputType) {
	case MOVEMENT:
		scene->handlePlayerMovement(client_id, packet->finalLocation);
		break;
	case SKILL:
		scene->handlePlayerSkill(client_id, 
			                     packet->finalLocation, 
			                     packet->skill_id,
			                     skill_map,
			                     player_metadata);
		break;
	case RESPAWN:
		scene->handlePlayerRespawn(client_id);
		break;
	default:
		break;
	}
	free(packet);
}