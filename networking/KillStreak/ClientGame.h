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

	int char_select_time;				      // time allotted to make character selection
	ServerInputQueue* serverPackets;	// queue of packets from server
	mutex* q_lock;						        // lock for queue
  
	vector<string> skill_names;
	vector<int> cooldown_times;

};