#pragma once
#include <winsock2.h>
#include "ClientNetwork.hpp"
#include "INIReader.h"
#include "Logger.hpp"
#include "CoreTypes.hpp"

class ClientGame {
public:
	ClientGame(INIReader& t_config);
	~ClientGame(void);

	ClientNetwork * network;

	void run();
	int join_game();

protected:
	INIReader & config;
	PCSTR host;
	PCSTR serverPort;
};