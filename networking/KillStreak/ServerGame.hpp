#pragma once
#ifndef SURFSTORESERVER_HPP
#define SURFSTORESERVER_HPP

#include "INIReader.h"
#include "logger.hpp"
#include "ServerNetwork.hpp"
#include <string>
#include <chrono>

using namespace std;

typedef chrono::high_resolution_clock Clock;
typedef chrono::nanoseconds nanoseconds;
typedef chrono::duration<double> dsec;

class ServerGame {
public:
	ServerGame(INIReader& t_config);

	void launch();

	void update();

	const int NUM_THREADS = 8;

protected:
	INIReader & config;
	PCSTR port;
	ServerNetwork* network;
	double tick_rate;
	static unsigned int client_id; // IDs for the clients connecting to the server in ServerNetwork
};

#endif // SURFSTORESERVER_HPP