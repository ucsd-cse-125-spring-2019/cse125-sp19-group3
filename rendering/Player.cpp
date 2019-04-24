#include "Player.h"

void Player::translate(glm::vec3 forward) {
	currentPos += forward * speed;
	playerRoot->translation = glm::translate(glm::mat4(1.0f), forward * speed) * playerRoot->translation;
	//printf("currentPos to: %f %f %f\n", currentPos.x, currentPos.y, currentPos.z);
}

void Player::rotate(float angle, glm::vec3 axis) {
	currentOri = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), angle, axis) * glm::vec4(currentOri, 0)));
	axis = glm::normalize(glm::vec3(glm::inverse(playerRoot->rotation) * glm::vec4(axis, 0)));
	printf("axis is: %f, %f, %f\n", axis.x, axis.y, axis.z);
	glm::mat4 rotM = glm::rotate(glm::mat4(1.0f), angle, axis);
	playerRoot->rotation = playerRoot->rotation * rotM;
	printf("current orientation is: %f, %f, %f\n", currentOri.x, currentOri.y, currentOri.z);
}

void Player::move() {
	//printf("dest::%f %f %f\n", destination.x, destination.y, destination.z);
	glm::vec3 forwardVector = destination - currentPos;
	if (glm::length(forwardVector) > 0.1f) {
		forwardVector = glm::normalize(forwardVector);
		//printf("forward to: %f %f %f\n", forwardVector.x, forwardVector.y, forwardVector.z);
		//moving forward :
		translate(forwardVector);
		//toWorld = orbit * toWorld;
	}
}

void Player::setDestination(glm::vec3 newDest) {
	destination = newDest;
}

void Player::update() {
	move();
	playerRoot->update();
}

