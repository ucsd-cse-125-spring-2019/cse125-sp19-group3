#pragma once
#include <winsock2.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ClientNetwork.hpp"
#include "Logger.hpp"
#include "CoreTypes.hpp"
#include "PlayerData.hpp"
#include <queue>

typedef enum { LOBBY, KILL, PREPARE } ClientStatus;
class ClientGame {
public:
	ClientGame(string host, string port, int char_select_time);
	//~ClientGame(void);

	ClientNetwork * network;
	nanoseconds prepareTimer = nanoseconds(std::chrono::seconds(10));
	LeaderBoard* leaderBoard;

	void run();
	int join_game();
	void handleServerInputPacket(ServerInputPacket* packet);
	int handleCharacterSelectionPacket(ServerInputPacket* packet);
	int sendCharacterSelection(string username, ArcheType character);
	int waitingInitScene();
	int waitingPrepareScene();
	// initialize packet 
	ClientInputPacket createClientInputPacket(InputType type, Point finalLocation, int skill_id);
	ClientInputPacket createMovementPacket(Point newLocation);
	ClientInputPacket createSkillPacket(Point destLocation, int skill_id);
	ClientInputPacket createRespawnPacket();
	ClientInputPacket createInitPacket();
	ClientSelectionPacket createCharacterSelectedPacket(std::string username, ArcheType type);

protected:
	PCSTR host;
	PCSTR serverPort;
	ClientStatus currPhase = PREPARE;
	int char_select_time;				      // time allotted to make character selection
	ServerInputQueue* serverPackets;	// queue of packets from server
	mutex* q_lock;						        // lock for queue
	vector<int> cooldown_times;

	friend class ClientScene;

};