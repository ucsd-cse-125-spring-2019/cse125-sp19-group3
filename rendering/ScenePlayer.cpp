#include "ScenePlayer.h"

ScenePlayer::ScenePlayer(unsigned int playerId, unsigned int playerRootId, ArcheType modelType, Transform * playerRoot, ServerScene * serverScene) {
	this->player_id = playerId;
	this->root_id = playerRootId;
	this->modelType = modelType;
	this->playerRoot = playerRoot;
	this->destination = this->currentPos = { playerRoot->translation[3][0], playerRoot->translation[3][1], playerRoot->translation[3][2] };
	this->action_state = ACTION_MOVEMENT;
	this->isSilenced = false;
	this->isEvading = false;
	this->serverScene = serverScene;
	auto skills = (*(serverScene->archetype_skillset))[modelType];
	for (unsigned int skill_id : skills) {
		Skill skill = (*(serverScene->skill_map))[skill_id];
		availableSkills.push_back(*(new Skill(skill_id, skill.level, skill.skillName, skill.range, skill.cooldown, skill.duration, skill.speed)));
	}
}

void ScenePlayer::translate(glm::vec3 forward) {
	currentPos += forward * speed;
	playerRoot->translation = glm::translate(glm::mat4(1.0f), forward * speed) * playerRoot->translation;
	//printf("currentPos to: %f %f %f\n", currentPos.x, currentPos.y, currentPos.z);
}

void ScenePlayer::rotate(float angle, glm::vec3 axis) {
	currentOri = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), angle, axis) * glm::vec4(currentOri, 0)));
	axis = glm::normalize(glm::vec3(glm::inverse(playerRoot->rotation) * glm::vec4(axis, 0)));
	// printf("axis is: %f, %f, %f\n", axis.x, axis.y, axis.z);
	glm::mat4 rotM = glm::rotate(glm::mat4(1.0f), angle, axis);
	playerRoot->rotation = playerRoot->rotation * rotM;
	// printf("current orientation is: %f, %f, %f\n", currentOri.x, currentOri.y, currentOri.z);
}

void ScenePlayer::move() {
	//printf("dest::%f %f %f\n", destination.x, destination.y, destination.z);
	glm::vec3 forwardVector = destination - currentPos;
	if (glm::length(forwardVector) > 0.2f) {
		forwardVector = glm::normalize(forwardVector);
		//printf("forward to: %f %f %f\n", forwardVector.x, forwardVector.y, forwardVector.z);
		//moving forward :
		translate(forwardVector);
		animationMode = run;
	}
	else {
		if (modelType == WARRIOR && warriorIsChargingServer) {
			serverScene->warriorIsDoneCharging = true;
			warriorIsChargingServer = false;
			speed = 0.3f; // hardcoding bs
		}
		animationMode = idle;
	}
}

void ScenePlayer::setDestination(glm::vec3 newDest) {
	destination = newDest;
}

void ScenePlayer::update() {
	move();
	playerRoot->update();
}

void ScenePlayer::animate(double currTime) {
	//model->BoneTransform(string("idlerunning"), currTime);
}
