#pragma once
#include <glm/glm.hpp>
#include <chrono>
#include <queue>
#include "ServerNetwork.hpp"


typedef glm::vec3 Point;

typedef enum {MOVEMENT, SKILL, ATTACK} InputType;
typedef struct {
	InputType inputType;
	// Movement Data
	Point finalLocation;

	// Skill Data
	int skillType; // indexes to which skill to access

	// Attack Data
	int attackType; // 0: melee, 1: projectile
} ClientInputPacket;

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::nanoseconds nanoseconds;
typedef std::chrono::duration<double> dsec;

typedef std::queue<ClientInputPacket> ClientThreadQueue;