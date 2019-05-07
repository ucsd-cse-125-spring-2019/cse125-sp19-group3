#include "ServerScene.h"

ServerScene::ServerScene()
{
	root = new Transform(glm::mat4(1.0f));
}

ServerScene::~ServerScene() {
	delete root;
}

void ServerScene::addPlayer(unsigned int playerId) {
	nodeIdCounter++;
	Transform * playerRoot = new Transform(glm::translate(glm::mat4(1.0f), glm::vec3(playerId * 10.0f, 0, 0)),
		glm::rotate(glm::mat4(1.0f), 0 * glm::pi<float>(), glm::vec3(1, 0, 0)),//-90 / 180.0f * glm::pi<float>(), glm::vec3(1, 0, 0)),
		glm::scale(glm::mat4(1.0f), glm::vec3(10.0f)));
		//glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)));
	playerRoot->model_ids.insert(PLAYER);
	root->addChild(nodeIdCounter, playerRoot);


	// TODO: need to set model type based on player selection in lobby

	Player * player = new Player(playerId, nodeIdCounter, HUMAN, playerRoot);
	//playerMap.insert(std::pair<unsigned int, Player *>(playerId, player));
	players.push_back(player);
}

void ServerScene::update()
{
	time += 1.0 / 60;
	for (Player * player : players) {
		player->update();
	}
}

void ServerScene::handlePlayerMovement(unsigned int playerId, glm::vec3 destination)
{
	Player * player = players[playerId];
	player->setDestination(destination);
	float dotResult = glm::dot(glm::normalize(destination - player->currentPos), player->currentOri);

	if (abs(dotResult) < 1.0) {
		float angle = glm::acos(dotResult);
		printf("rotate angle = %f", angle);
		glm::vec3 axis = glm::cross(player->currentOri, glm::normalize(destination - player->currentPos));
		if (glm::length(axis) != 0) {
			player->rotate(angle, axis);
		}
	}
}

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

std::pair<char *, unsigned int> ServerScene::serializeSceneGraph(Transform* t, char* data) {
	memcpy(data, &(t->M[0][0]), sizeof(glm::mat4));
	data += sizeof(glm::mat4);
	unsigned int size = sizeof(glm::mat4);

	for (auto child : t->children) {
		*data++ = 'T';
		size += sizeof(char);
		memcpy(data, &child.first, sizeof(unsigned int));
		data += sizeof(unsigned int);
		size += sizeof(unsigned int);
		auto retval = serializeSceneGraph(child.second, data);
		data = retval.first;
		size += retval.second;
	}

	for (auto model_id : t->model_ids) {
		*data++ = 'M';
		size += sizeof(char);
		memcpy(data, &model_id, sizeof(unsigned int));
		data += sizeof(unsigned int);
		size += sizeof(unsigned int);
	}

	//*data++ = '\0';
	//size += sizeof(char);
	return std::pair<char *, unsigned int>(data, size);
}
