#pragma once
#include <glm/glm.hpp>
#include <chrono>
#include <queue>

#define NULL_POINT Point(0.0,0.0,0.0)

typedef glm::vec3 Point;

typedef enum {INIT_CONN, MOVEMENT, SKILL, ATTACK, GAME_STATE} InputType;

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
	int game_state;

} ServerInputPacket;


typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::nanoseconds nanoseconds;
typedef std::chrono::duration<double> dsec;

typedef std::queue<ClientInputPacket> ClientThreadQueue;
