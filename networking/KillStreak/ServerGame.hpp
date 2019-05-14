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
	unsigned int id;				// client ID
	ClientThreadQueue *q_ptr;		// queue pointer
	ServerNetwork *network;			// Server network pointer
	mutex* q_lock;					// lock for client queue
} client_data;

class ServerGame {
public:
	ServerGame(string host, string port, double tick_rate);

	void launch();

	void updateKillPhase();

	void updatePreparePhase();

	void game_match();

	const int NUM_THREADS = 8;

protected:
	PCSTR host;
	PCSTR port;
  
	double tick_rate;
	int char_select_time;					// time for character selection phase

	vector<client_data*> client_data_list;	// list of pointers to all client data (queue, id, lock, etc.)
	ScheduledEvent scheduledEvent;
  
	unordered_map<unsigned int, PlayerMetadata> playerMetadatas;	// map client_id to player meta_data
	unordered_map<ArcheType, vector<Skill>> skill_map;				// Map ArchType to list of skills

	ServerScene * scene;

	ServerNetwork* network;					// ptr to servers network

	void readMetaDataForSkills();

	// create packet for server to send to client
	ServerInputPacket createServerPacket(ServerPacketType type, int temp, char* data);
	ServerInputPacket createInitScenePacket(unsigned int playerId, unsigned int playerRootid);
	ServerInputPacket createServerTickPacket();
	ServerInputPacket createWelcomePacket();
	ServerInputPacket createCharSelectPacket();

	
	void handleClientInputPacket(ClientInputPacket* packet, int client_id);
};

#endif // SURFSTORESERVER_HPP