#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "Cube.h"
#include "Model.h"
#include "Transform.h"

class Player {
public:
	Player() {};
	Player(Transform * playerRoot, Model * model) { this->playerRoot = playerRoot; this->model = model; };
	~Player();

	void move();
	void rotate(float angle, glm::vec3 axis);
	void setDestination(glm::vec3 newDest);
	void translate(glm::vec3 forward);
	void update();

	Transform * playerRoot;
	Model * model;
	glm::vec3 destination = glm::vec3(0.0f);
	glm::vec3 currentPos;
	glm::vec3 currentOri;
	float speed = 0.2f;
};

#endif