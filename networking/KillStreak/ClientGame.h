#pragma once
#include <winsock2.h>
#include "ClientNetwork.hpp"
#include "Logger.hpp"
#include "CoreTypes.hpp"
#include "PlayerData.hpp"
#include <queue>

class ClientGame {
public:
	ClientGame(string host, string port, int char_select_time);
	//~ClientGame(void);

	ClientNetwork * network;

	void run();
	int join_game();
	void handleServerInputPacket(ServerInputPacket* packet);
	int handleCharacterSelectionPacket(ServerInputPacket* packet);

	// initialize packet 
	ClientInputPacket createClientInputPacket(InputType type, Point initLocation, Point finalLocation,
		int skill_id);
	ClientInputPacket createMovementPacket(Point newLocation);
	ClientInputPacket createProjectilePacket(Point initLocation, Point newLocation);
	ClientInputPacket createMageOmniAoePacket();
	ClientInputPacket createMageDirectionalAoePacket(Point initLocation, Point newLocation);
	ClientInputPacket createInitPacket();
	ClientSelectionPacket createCharacterSelectedPacket(std::string username, ArcheType type);

protected:
	PCSTR host;
	PCSTR serverPort;

	int char_select_time;				      // time allotted to make character selection
	ServerInputQueue* serverPackets;	// queue of packets from server
	mutex* q_lock;						        // lock for queue
	vector<int> cooldown_times;

};