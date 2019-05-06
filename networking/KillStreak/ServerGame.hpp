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
	ServerGame(INIReader& t_config);

	void launch();

	void updateKillPhase();

	void updatePreparePhase();

	void game_match();

	const int NUM_THREADS = 8;

protected:
	INIReader & config;
	PCSTR host;
	PCSTR port;
	ServerNetwork* network;
	double tick_rate;
	vector<client_data*> client_data_list;	// list of pointers to all client meta-data
	ScheduledEvent scheduledEvent;

	ServerScene * scene;


	ServerInputPacket createInitScenePacket(unsigned int playerId, unsigned int playerRootid);
	ServerInputPacket createServerTickPacket();
};

#endif // SURFSTORESERVER_HPP