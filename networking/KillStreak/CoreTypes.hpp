#pragma once
#include <glm/glm.hpp>
#include <chrono>
#include <queue>

#define NULL_POINT Point(0.0,0.0,0.0)

typedef glm::vec3 Point;

typedef enum {INIT_CONN, MOVEMENT, SKILL, ATTACK} InputType;

/*
	Packet send from the client to the server.
*/
typedef struct {
	InputType inputType;
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
	InputType inputType;
	int temp;
	// TODO: Add data!!! 
} ServerInputPacket;


typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::nanoseconds nanoseconds;
typedef std::chrono::duration<double> dsec;

typedef std::queue<ClientInputPacket> ClientThreadQueue;

/* Server scheduling queue event */
typedef enum {END_KILLPHASE, END_PREPAREPHASE} EventType;

typedef struct ScheduledEvent {
	EventType eventType;
	int ticksLeft; // assume that all ticks are constant amount
	ScheduledEvent(EventType et, int tl) {
		eventType = et;
		ticksLeft = tl;
	}
} ScheduledEvent;