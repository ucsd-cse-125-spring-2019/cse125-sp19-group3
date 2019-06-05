#pragma once
#include <glm/glm.hpp>
#include <chrono>
#include <queue>
#include <string>

#define GAME_SIZE				2			// total players required to start game
#define NULL_POINT				Point(0.0,0.0,0.0)

#define SERVER_TICK_PACKET_SIZE 10000
#define END_PHASE_PACKET_SIZE   512

#define KILLPHASE_TIME			10			// duration of kill phase
#define PREPHASE_TIME			15			// duration of prepare phase
#define ENDGAME_TIME			5			// duration of end of game 
#define TOTAL_ROUNDS			3			// total rounds to be played in game

typedef glm::vec3 Point;

typedef enum {INIT_CONN, CHAR_SELECT, MOVEMENT, SKILL, RESPAWN, END_KILL_PHASE, END_PREP_PHASE } InputType;

typedef enum { WELCOME, INIT_SCENE, UPDATE_SCENE_GRAPH, 
				CHAR_SELECT_PHASE, START_PREP_PHASE, START_KILL_PHASE, START_END_GAME_PHASE } ServerPacketType;

typedef enum { HUMAN, MAGE, ASSASSIN, WARRIOR, KING } ArcheType;

enum AnimationType { idle, run, evade, projectile, skill_1, skill_2, die, spawn };

/*
	Packet sent from the client to the server when client 
	selects username and character in lobby.
*/
typedef struct {
	InputType inputType;
	std::string username;
	ArcheType type;
} ClientSelectionPacket;


/*
	Packet sent from the client to the server when client wants to 
	tell the server their prep phase is over... start kill phase!!!

	NOTE: COULDN'T USE THIS... server queue is of type ClientInputPacket...
typedef struct {
	InputType inputType;
	int size;
	char data[END_PHASE_PACKET_SIZE]; 
} ClientStartKillPhasePacket;

*/

/*
	Packet sent from the client to the server.
*/
typedef struct {
	InputType inputType;
	Point finalLocation;
	int skill_id; // indexes to which skill to access
	int size;
	char data[END_PHASE_PACKET_SIZE];
} ClientInputPacket;


/*
	Packet sent from the server to the client.
*/
typedef struct {
	ServerPacketType packetType;
	int size;
	char data[SERVER_TICK_PACKET_SIZE];
	bool died_this_tick;	// true only on server tick client died
} ServerInputPacket;


typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::nanoseconds nanoseconds;
typedef std::chrono::duration<double> dsec;

// queue for each client threads packets (server)
typedef std::queue<ClientInputPacket*> ClientThreadQueue;	
// queue for clients incoming packets from server (client)
typedef std::queue<ServerInputPacket*> ServerInputQueue;    

/* Server scheduling queue event */
typedef enum {END_KILLPHASE, END_PREPAREPHASE} EventType;

typedef struct ScheduledEvent {
	EventType eventType;
	int ticksLeft; // assume that all ticks are constant amount
	ScheduledEvent(EventType et, int tl) {
		eventType = et;
		ticksLeft = tl;
	};
	ScheduledEvent() {
		eventType = END_KILLPHASE;
		ticksLeft = 10000000000;
	}
} ScheduledEvent;