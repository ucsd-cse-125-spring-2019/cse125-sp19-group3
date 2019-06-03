#pragma once
#ifndef SURFSTORESERVER_HPP
#define SURFSTORESERVER_HPP

#include "INIReader.h"
#include "logger.hpp"
#include "ServerNetwork.hpp"
#include "CoreTypes.hpp"
#include "../../ServerScene.h"
#include <string>
#include <vector>
#include <mutex>

using namespace std;


/*
Contains meta data for client passed to client thread.
*/
typedef struct {

	// client id
	unsigned int id;				

	// queue pointer and corresponding lock
	ClientThreadQueue* q_ptr;
	mutex* q_lock;

	// Server network pointer
	ServerNetwork* network;

	// pointers to servers maps
	unordered_map<unsigned int, Skill> *skill_map_ptr;					
	unordered_map<ArcheType, int> *selected_chars_map_ptr;
	unordered_map<unsigned int, PlayerMetadata*> *playerMetadatas_ptr;		
	unordered_map<ArcheType, vector<unsigned int>> *archetype_skillset_ptr;

	// selected character mapping pointer & corresponding lock
	mutex* char_lock;


} client_data;

class ServerGame {
public:
	ServerGame(string host, string port, double tick_rate);

	void launch();

	void game_match();

	bool updateKillPhase();

	void updatePreparePhase();

	void launch_client_threads();

	const int NUM_THREADS = 8;

	int end_kill_phase = 0;			// true if any client initiated end_kill_phase
	int total_end_kill_packets = 0; // number of end_kill_phase packets received from clients 

protected:
	PCSTR host;
	PCSTR port;
  
	double tick_rate;

	vector<client_data*> client_data_list;	// list of pointers to all client data (queue, id, lock, etc.)
	ScheduledEvent scheduledEvent;

	LeaderBoard* leaderBoard;
  
	unordered_map<unsigned int, Skill> *skill_map;								// Map ArchType to list of skills
	unordered_map<unsigned int, PlayerMetadata*> *playerMetadatas;				// map client_id to player meta_data
	unordered_map<ArcheType, int> *selected_characters;							// Map selected character to client_id
	unordered_map<ArcheType, vector<unsigned int>> *archetype_skillset;

	vector<int> archetypes;					// list of all client archetypes indexed by client id
	vector<string> usernames;				// list of all client usernames indexed by client id

	ServerScene * scene;
	ServerNetwork* network;					// ptr to servers network
	mutex* char_select_lock;				// lock for character selection

	void readMetaDataForSkills();

	// create packet for server to send to client
	ServerInputPacket createServerPacket(ServerPacketType type, int temp, char* data);
	ServerInputPacket createInitScenePacket(unsigned int playerId, unsigned int playerRootid);
	ServerInputPacket createStartPrepPhasePacket();
	ServerInputPacket createServerTickPacket();
	ServerInputPacket createWelcomePacket();
	ServerInputPacket createCharSelectPacket(char* data, int size);
	
	int handleClientInputPacket(ClientInputPacket* packet, int client_id);

	friend class ScenePlayer;
};

#endif // SURFSTORESERVER_HPP