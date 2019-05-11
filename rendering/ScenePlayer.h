#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "Cube.h"
#include "Model.h"
#include "Transform.h"
#include "../networking/KillStreak/CoreTypes.hpp"

typedef enum {HUMAN_MODEL, MAGE_MODEL, WARRIOR_MODEL, ASSASIN_MODEL} MODEL_TYPE;

class ScenePlayer {
public:
	ScenePlayer() {};
	ScenePlayer(unsigned int playerId, unsigned int playerRootId, ArcheType modelType, Transform * playerRoot) 
	{
		this->player_id = playerId; this->root_id = playerRootId; this->modelType = modelType; this->playerRoot = playerRoot; 
		this->destination = this->currentPos = { playerRoot->translation[3][0], playerRoot->translation[3][1], playerRoot->translation[3][2] };
	};
	~ScenePlayer() {};

	void move();
	void rotate(float angle, glm::vec3 axis);
	void setDestination(glm::vec3 newDest);
	void translate(glm::vec3 forward);
	void update();
	void animate(double currTime);
	unsigned int player_id;
	unsigned int root_id;
	ArcheType modelType;
	Transform * playerRoot;
	//Model * model;
	glm::vec3 destination = glm::vec3(0.0f);
	glm::vec3 currentPos = glm::vec3(0.0f);
	glm::vec3 currentOri = glm::vec3(0.0f, 0.0f, 1.0f);
	float speed = 0.3f;
};

#endif