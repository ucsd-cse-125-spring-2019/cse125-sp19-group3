#pragma once
#include <winsock2.h>
#include "ClientNetwork.hpp"
#include "INIReader.h"
#include "Logger.hpp"
#include "CoreTypes.hpp"
#include <queue>

class ClientGame {
public:
	ClientGame(INIReader& t_config);
	~ClientGame(void);

	ClientNetwork * network;

	void run();
	int join_game();
	void sendPacket(InputType inputType, Point finalLocation, int skillType, int attackType);

protected:
	INIReader & config;
	PCSTR host;
	PCSTR serverPort;
	ServerInputQueue serverPackets;
};