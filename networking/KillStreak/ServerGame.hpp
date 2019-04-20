#pragma once
#ifndef SURFSTORESERVER_HPP
#define SURFSTORESERVER_HPP

#include "INIReader.h"
#include "logger.hpp"
#include "ServerNetwork.hpp"
#include <string>
#include <chrono>
#include <queue>

using namespace std;

typedef chrono::high_resolution_clock Clock;
typedef chrono::nanoseconds nanoseconds;
typedef chrono::duration<double> dsec;
typedef queue<char*> MasterQueue;


/*
	Contains meta data for client passed to client thread. 
*/
typedef struct {
	unsigned int id;			// client ID
	MasterQueue *mq_ptr;		// master queue pointer
	ServerNetwork *network;		// Server network pointer
}client_data;


class ServerGame {
public:
	ServerGame(INIReader& t_config);

	void launch();

	void update();

	void game_match(MasterQueue *mq);

	const int NUM_THREADS = 8;

protected:
	INIReader & config;
	PCSTR host;
	PCSTR port;
	ServerNetwork* network;
	double tick_rate;
};

#endif // SURFSTORESERVER_HPP