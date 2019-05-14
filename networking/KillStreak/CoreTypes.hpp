#pragma once
#include <glm/glm.hpp>
#include <chrono>
#include <queue>
#include <string>

#define NULL_POINT Point(0.0,0.0,0.0)
#define SERVER_TICK_PACKET_SIZE 10000

typedef glm::vec3 Point;

typedef enum {INIT_CONN, CHAR_SELECT, MOVEMENT, SKILL_PROJECTILE, ATTACK} InputType;

typedef enum { WELCOME, INIT_SCENE, UPDATE_SCENE_GRAPH, CHAR_SELECT_PHASE } ServerPacketType;

typedef enum { HUMAN, MAGE, ASSASSIN, WARRIOR } ArcheType;


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
	Packet sent from the client to the server.
*/
typedef struct {
	InputType inputType;
	Point initialLocation;
	// Movement Data
	Point finalLocation;

	// Skill Data
	int skillType; // indexes to which skill to access

	// Attack Data
	int attackType; // 0: melee, 1: projectile
} ClientInputPacket;


/*
	Packet sent from the server to the client.
*/
typedef struct {
	ServerPacketType packetType;
	int size;
	char data[SERVER_TICK_PACKET_SIZE];
	// TODO: Add data!!! 
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