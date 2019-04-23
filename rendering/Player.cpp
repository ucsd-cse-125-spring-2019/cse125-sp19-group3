#include "Player.h"

void Player::translate(glm::vec3 forward) {
	currentPos += forward * speed;
	playerRoot->M = glm::translate(glm::mat4(1.0f), forward * speed) * playerRoot->M;
	printf("currentPos to: %f %f %f\n", currentPos.x, currentPos.y, currentPos.z);
}

void Player::rotate(float angle, glm::vec3 axis) {
	glm::mat4 rotM = glm::rotate(glm::mat4(1.0f), angle, axis);
	playerRoot->M = glm::translate(glm::mat4(1.0f), currentPos) * rotM * glm::translate(glm::mat4(1.0f), -currentPos) * playerRoot->M;
	currentOri = glm::vec3(rotM * glm::vec4(currentOri, 0));
}

void Player::move() {
	printf("dest::%f %f %f\n", destination.x, destination.y, destination.z);
	glm::vec3 forwardVector = destination - currentPos;
	if (glm::length(forwardVector) > 0.1f) {
		forwardVector = glm::normalize(forwardVector);

		printf("forward to: %f %f %f\n", forwardVector.x, forwardVector.y, forwardVector.z);

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
}

