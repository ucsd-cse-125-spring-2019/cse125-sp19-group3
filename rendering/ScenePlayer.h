#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "Cube.h"
#include "Model.h"
#include "Transform.h"
#include "../networking/KillStreak/CoreTypes.hpp"
#include "../networking/KillStreak/PlayerData.hpp"
#include "../rendering/ServerScene.h"

// typedef enum {HUMAN_MODEL, MAGE_MODEL, WARRIOR_MODEL, ASSASIN_MODEL} MODEL_TYPE;
typedef enum {ACTION_MOVEMENT, ACTION_DIRECTIONAL_SKILL} ACTION_STATE;		// client moving or in projectile mode? 

class ServerScene;

class ScenePlayer {
public:
	ScenePlayer() {};
	ScenePlayer(unsigned int playerId, unsigned int playerRootId, ArcheType modelType, Transform * playerRoot, ServerScene * serverScene);
	~ScenePlayer() {};

	void move();
	void rotate(float angle, glm::vec3 axis);
	void setDestination(glm::vec3 newDest);
	void translate(glm::vec3 forward);
	void update();
	void animate(double currTime);
	unsigned int player_id;
	unsigned int root_id;

	unsigned int animationMode = idle;
	ArcheType modelType;
	Transform * playerRoot;
	ServerScene * serverScene = NULL;
	//Model * model;
	glm::vec3 destination = glm::vec3(0.0f);
	glm::vec3 currentPos = glm::vec3(0.0f);
	glm::vec3 currentOri = glm::vec3(0.0f, 0.0f, 1.0f);
	float speed = 0.3f;
	ACTION_STATE action_state;
	bool isPrepProjectile;
	bool isSilenced;
	bool isEvading;
	bool warriorIsChargingServer = false;
	vector<Skill> availableSkills;

};

#endif