#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "Cube.h"
#include "Model.h"
#include "Transform.h"

typedef enum {HUMAN, MAGE, WARRIOR, ASSASIN} MODEL_TYPE;

class Player {
public:
	Player() {};
	Player(unsigned int playerId, unsigned int playerRootId, MODEL_TYPE modelType) { this->player_id = playerId; this->root_id = playerRootId; this->modelType = modelType; };
	~Player() {};

	void move();
	void rotate(float angle, glm::vec3 axis);
	void setDestination(glm::vec3 newDest);
	void translate(glm::vec3 forward);
	void update(double currTime);

	unsigned int player_id;
	unsigned int root_id;
	MODEL_TYPE modelType;
	Transform * playerRoot;
	Model * model;
	glm::vec3 destination = glm::vec3(0.0f);
	glm::vec3 currentPos = glm::vec3(0.0f);
	glm::vec3 currentOri = glm::vec3(0.0f, 0.0f, 1.0f);
	float speed = 0.2f;
};

#endif