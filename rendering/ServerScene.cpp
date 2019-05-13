#include "ServerScene.h"
#include "nlohmann\json.hpp"
#include <fstream>

using json = nlohmann::json;

ServerScene::ServerScene()
{
	root = new Transform(0, glm::mat4(1.0f));
	serverSceneGraphMap.insert({ root->node_id, root });
	nodeIdCounter++;
	playerRoot = new Transform(nodeIdCounter, glm::mat4(1.0f));
	serverSceneGraphMap.insert({ playerRoot->node_id, playerRoot });
	root->addChild(nodeIdCounter);
	nodeIdCounter++;
	skillRoot = new Transform(nodeIdCounter, glm::mat4(1.0f));
	serverSceneGraphMap.insert({ skillRoot->node_id, skillRoot });
	root->addChild(nodeIdCounter);

	initModelRadius();
}

ServerScene::~ServerScene() {
	delete root;
}

void ServerScene::initModelRadius() {
	ifstream json_file("../model_paths.json");
	json jsonObjs = json::parse(json_file);
	for (auto & obj : jsonObjs["data"]) {
		model_radius.insert({ (unsigned int)obj["model_id"], (float)obj["radius"] });
	}
}


void ServerScene::initEnv() {
	ifstream json_file("../env_model_locations.json");
	unsigned int envCounter = 4000000000;
	json jsonObjs = json::parse(json_file);
	for (auto & obj : jsonObjs["data"]) {
		Transform * envobj = new Transform(envCounter++, glm::translate(glm::mat4(1.0f), glm::vec3((float)(obj["translate"][0]), (float)(obj["translate"][1]), (float)(obj["translate"][2]))),
		glm::rotate(glm::mat4(1.0f), (float)obj["rotate"] / 180.0f * glm::pi<float>(), glm::vec3(0, 1, 0)),
		glm::scale(glm::mat4(1.0f), glm::vec3((float)obj["scale"], (float)obj["scale"], (float)obj["scale"])));
		
		envobj->model_ids.insert((int)obj["model_id"]);
		env_objs.push_back(envobj);
	}
}

void ServerScene::addPlayer(unsigned int playerId, ArcheType modelType) {
	nodeIdCounter++;
	Transform * playerObj = new Transform(nodeIdCounter, glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)),
		glm::rotate(glm::mat4(1.0f), -90/ 180.0f * glm::pi<float>(), glm::vec3(1, 0, 0)),
		glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)));
	playerObj->model_ids.insert(modelType);
	playerRoot->addChild(nodeIdCounter);
	serverSceneGraphMap.insert({ nodeIdCounter, playerObj });

	// TODO: need to set model type based on player selection in lobby

	ScenePlayer player = ScenePlayer(playerId, nodeIdCounter, modelType, playerObj);
	//playerMap.insert(std::pair<unsigned int, Player *>(playerId, player));
	scenePlayers.insert({ playerId, player });
}

void ServerScene::update()
{
	time += 1.0 / 60;
	for (auto &element : scenePlayers) {
		checkAndHandleCollision(element.first);
		element.second.update();
		
	}
}

//TODO: Refactoring, moving collision check to player??? Also find radius for various objs.
void ServerScene::checkAndHandleCollision(unsigned int playerId) {
	ScenePlayer &player = scenePlayers[playerId];
	for (auto& envObj : env_objs) {
		glm::vec3 forwardVector = glm::normalize(player.destination - player.currentPos)* player.speed;
		if (player.playerRoot->isCollided(forwardVector, model_radius, serverSceneGraphMap, envObj)) {
			player.setDestination(player.currentPos);
			break;
		}
	}

}

void ServerScene::handlePlayerMovement(unsigned int playerId, glm::vec3 destination)
{
	ScenePlayer &player = scenePlayers[playerId];
	player.setDestination(destination);
	float dotResult = glm::dot(glm::normalize(destination - player.currentPos), player.currentOri);

	if (abs(dotResult) < 1.0) {
		float angle = glm::acos(dotResult);
		printf("rotate angle = %f", angle);
		glm::vec3 axis = glm::cross(player.currentOri, glm::normalize(destination - player.currentPos));
		if (glm::length(axis) != 0) {
			player.rotate(angle, axis);
		}
	}
}

// TODO: Complete me!!!
void ServerScene::handlePlayerProjectile()
{
	return;
}


Transform * ServerScene::getRoot() {
	return root;
}

/*
unsigned int ServerScene::serializeInitScene(char* data, unsigned int playerId, unsigned int playerRootId) {
	memcpy(data, &playerId, sizeof(unsigned int));
	data += sizeof(unsigned int);
	memcpy(data, &playerRootId, sizeof(unsigned int));
	data += sizeof(unsigned int);
	return 2 * sizeof(unsigned int) + serializeSceneGraph(data);

}

unsigned int ServerScene::serializeSceneGraph(char* data) {
	return serializeSceneGraph(root, data).second;
}

//std::pair<char *, unsigned int> ServerScene::serializeSceneGraph(Transform* t, char* data) {
//	memcpy(data, &(t->M[0][0]), sizeof(glm::mat4));
//	data += sizeof(glm::mat4);
//	unsigned int size = sizeof(glm::mat4);
//
//	for (auto child : t->children) {
//		*data++ = 'T';
//		size += sizeof(char);
//		memcpy(data, &child.first, sizeof(unsigned int));
//		data += sizeof(unsigned int);
//		size += sizeof(unsigned int);
//		auto retval = serializeSceneGraph(child.second, data);
//		data = retval.first;
//		size += retval.second;
//	}
//
//	for (auto model_id : t->model_ids) {
//		*data++ = 'M';
//		size += sizeof(char);
//		memcpy(data, &model_id, sizeof(unsigned int));
//		data += sizeof(unsigned int);
//		size += sizeof(unsigned int);
//	}
//
//	//*data++ = '\0';
//	//size += sizeof(char);
//	return std::pair<char *, unsigned int>(data, size);
//}


std::pair<char *, unsigned int> ServerScene::serializeSceneGraph(Transform* t, char* data) {
	std::queue<Transform *> toSerialize;
	toSerialize.push(t);
	unsigned int size = 0;
	while (!toSerialize.empty()) {
		Transform * currNode = toSerialize.front();
		toSerialize.pop();
		unsigned int serialized_size = currNode->serialize(data);
		data += serialized_size;
		size += serialized_size;
		for (auto child : currNode->children) {
			toSerialize.push(child.second);
		}
	}
	return std::pair<char *, unsigned int>(data, size);
}
*/
