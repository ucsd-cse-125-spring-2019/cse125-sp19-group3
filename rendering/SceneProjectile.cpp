#include "SceneProjectile.h"

void SceneProjectile::translate(Point forward) {
	currentPos += forward * speed;
	node->translation = glm::translate(glm::mat4(1.0f), forward * speed) * node->translation;
}

void SceneProjectile::rotate(float angle, Point axis) {
	currentOri = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), angle, axis) * glm::vec4(currentOri, 0)));
	axis = glm::normalize(glm::vec3(glm::inverse(node->rotation) * glm::vec4(axis, 0)));
	glm::mat4 rotM = glm::rotate(glm::mat4(1.0f), angle, axis);
	node->rotation = node->rotation * rotM;
}

void SceneProjectile::move() {
	translate(direction);
}

void SceneProjectile::update() {
	if (!isAOE) {
		move();
		node->update();
	}
	else {
		node->scale = glm::scale(glm::mat4(1.0f), glm::vec3(node->scale[0][0] + speed));
		node->update();
	}
}

bool SceneProjectile::outOfRange() {
	return isAOE ? node->scale[0][0] >= range : glm::length(currentPos - initPos) > range;
}

