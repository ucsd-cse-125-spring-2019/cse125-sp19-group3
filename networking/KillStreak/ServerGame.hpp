#pragma once
#ifndef SURFSTORESERVER_HPP
#define SURFSTORESERVER_HPP

#include "INIReader.h"
#include "logger.hpp"
#include "ServerNetwork.hpp"
#include "CoreTypes.hpp"
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
	mutex* lock;
} client_data;

class ServerGame {
public:
	ServerGame(INIReader& t_config);

	void launch();

	void update();

	void game_match();

	const int NUM_THREADS = 8;

protected:
	INIReader & config;
	PCSTR host;
	PCSTR port;
	ServerNetwork* network;
	double tick_rate;
	vector<ClientThreadQueue*> clientThreadQueues;
	vector<mutex*> locks;
};

#endif // SURFSTORESERVER_HPP