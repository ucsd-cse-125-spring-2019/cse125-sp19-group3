#include "Serialization.h"
#include "../networking/KillStreak/Logger.hpp"


void removeSubtreeInSceneGraph(const unsigned int node_id, unordered_map<unsigned int, Transform *> &sceneGraphMap) {
	Transform * node = sceneGraphMap[node_id];
	for (auto child : node->children_ids) {
		removeSubtreeInSceneGraph(child, sceneGraphMap);
	}
	delete node;
	sceneGraphMap.erase(node_id);
	return;
}


unsigned int Serialization::serializeSceneGraph(Transform * node, char *data, unordered_map<unsigned int, Transform *> &serverSceneGraphMap) {
	// first serialize # of nodes
	unsigned int numNodes;
	unsigned int size = 0;
	numNodes = serverSceneGraphMap.size();
	memcpy(data, &numNodes, sizeof(unsigned int));
	data += sizeof(unsigned int);
	size += sizeof(unsigned int);
	std::queue<Transform *> toSerialize;
	toSerialize.push(node);
	while (!toSerialize.empty()) {
		Transform * currNode = toSerialize.front();
		toSerialize.pop();
		unsigned int serialized_size = currNode->serialize(data);
		data += serialized_size;
		size += serialized_size;
		for (auto child_id : currNode->children_ids) {
			auto child = serverSceneGraphMap[child_id];
			toSerialize.push(child);
		}
	}
	
	return size;
}

// Serialize the players' animation mode
unsigned int Serialization::serializeAnimationMode(unordered_map<unsigned int, ScenePlayer> &scenePlayers, char *data) {
	unsigned int numPlayers = scenePlayers.size();
	unsigned int size = 0;
	memcpy(data, &numPlayers, sizeof(unsigned int));
	data += sizeof(unsigned int);
	size += sizeof(unsigned int);
	for (auto &p : scenePlayers) {
		unsigned int modelId = p.second.modelType;
		int movementMode = p.second.movementMode;
		int animationMode = p.second.animationMode;
		memcpy(data, &modelId, sizeof(unsigned int));
		data += sizeof(unsigned int);
		size += sizeof(unsigned int);
		memcpy(data, &movementMode, sizeof(int));
		data += sizeof(int);
		size += sizeof(int);
		memcpy(data, &animationMode, sizeof(int));
		data += sizeof(int);
		size += sizeof(int);
		if (animationMode != -1) {
			logger()->debug("CEREAL: animation mode is {}", animationMode);
		}
		// always set animationMode back to -1
		p.second.animationMode = -1;
	}
	return size;
}

// deserialize one of the leaderbo// serialize leaderboard
unsigned int Serialization::serializeLeaderBoard(char* lb_data, LeaderBoard* leaderBoard)
{
	unsigned int size = 0;
	for (int i = 0; i < GAME_SIZE; i++)			// kills
	{
		memcpy(lb_data, &leaderBoard->currentKills[i], sizeof(int));
		size += sizeof(int);
		lb_data += sizeof(int);
	}
	for (int i = 0; i < GAME_SIZE; i++)			// points
	{
		memcpy(lb_data, &leaderBoard->currPoints[i], sizeof(int));
		size += sizeof(int);
		lb_data += sizeof(int);
	}
	for (int i = 0; i < GAME_SIZE; i++)			// prizes
	{
		memcpy(lb_data, &leaderBoard->prizes[i], sizeof(int));
		size += sizeof(int);
		lb_data += sizeof(int);
	}

	memcpy(lb_data, &leaderBoard->prizeChange, sizeof(float));
	size += sizeof(float);

	return size;
}

// deserialize leaderboard
unsigned int Serialization::deserializeLeaderBoard(char* lb_data, LeaderBoard* leaderBoard)
{
	unsigned int sz = 0;

	for (int i = 0; i < GAME_SIZE; i++)		// kills
	{
		memcpy(&leaderBoard->currentKills[i], lb_data, sizeof(int));
		lb_data += sizeof(int);
		sz += sizeof(int);
	}
	for (int i = 0; i < GAME_SIZE; i++)		// points
	{
		memcpy(&leaderBoard->currPoints[i], lb_data, sizeof(int));
		lb_data += sizeof(int);
		sz += sizeof(int);
	}
	for (int i = 0; i < GAME_SIZE; i++)		// prizes
	{
		memcpy(&leaderBoard->prizes[i], lb_data, sizeof(int));
		lb_data += sizeof(int);
		sz += sizeof(int);
	}

	memcpy(&leaderBoard->prizeChange, lb_data, sizeof(float));
	sz += sizeof(float);

	return sz;
}

// must make sure that the first field in a serialized node is the node_id
unsigned int Serialization::deserializeSingleNodeId(char *data) {
	unsigned int node_id;
	memcpy(&node_id, data, sizeof(unsigned int));
	return node_id;
}

// returns all elements that exist in set1 but do NOT exist in set2
unordered_set<unsigned int> compareSets(unordered_set<unsigned int> set1, unordered_set<unsigned int> set2) {
	unordered_set<unsigned int> difference;
	for (auto element : set1) {
		if (set2.count(element) == 0) {
			difference.insert(element);
		}
	}
	return difference;
}


Transform * Serialization::deserializeSceneGraph(char *data, unordered_map<unsigned int, Transform *> &clientSceneGraphMap, GLuint particleTexture, Shader * particleShader) {
	Transform * root = NULL;
	unsigned int numNodes;
	memcpy(&numNodes, data, sizeof(unsigned int));
	data += sizeof(unsigned int);
	for (int i = 0; i < numNodes; i++) {
		unsigned int node_id = deserializeSingleNodeId(data);
		unsigned int size;
		// exists in the map
		if (clientSceneGraphMap.count(node_id) != 0) {
			Transform * node = clientSceneGraphMap[node_id];
			std::unordered_set<unsigned int> oldChildren = node->children_ids;
			size = node->deserializeAndUpdate(data, particleShader, particleTexture);
			std::unordered_set<unsigned int> newChildren = node->children_ids;
			auto toDelete = compareSets(oldChildren, newChildren);
			for (auto element : toDelete) {
				removeSubtreeInSceneGraph(element, clientSceneGraphMap);
			}
			if (!root) {
				root = node;
			}
		}
		else {
			Transform * newNode = new Transform();
			size = newNode->deserializeAndUpdate(data, particleShader, particleTexture);
			clientSceneGraphMap.insert({ node_id, newNode });
			if (!root) {
				root = newNode;
			}
		}
		data += size;
	}
	return root;
}

// Serialize the players' animation mode
unsigned int Serialization::deserializeAnimationMode(char *data, unordered_map<unsigned int, vector<int>> &animationModes) { // TODO: maybe change to a map of vectors?
	unsigned int numPlayers;
	unsigned int size = 0;
	memcpy(&numPlayers, data, sizeof(unsigned int));
	size += sizeof(unsigned int);
	data += sizeof(unsigned int);
	for (unsigned int i = 0; i < numPlayers; i++) {
		unsigned int modelId;
		int movementMode;
		int animationMode;
		memcpy(&modelId, data, sizeof(unsigned int));
		data += sizeof(unsigned int);
		memcpy(&movementMode, data, sizeof(int));
		data += sizeof(int);
		memcpy(&animationMode, data, sizeof(int));
		data += sizeof(int);
		vector<int> modes;
		modes.push_back(movementMode);
		modes.push_back(animationMode);
		if (animationMode != -1) {
			logger()->debug("DESERIALIZE: animation mode is {}", animationMode);
		}
		animationModes.insert({modelId, modes});
		size += (sizeof(unsigned int) + sizeof(int) + sizeof(int));
	}
	return size;
}


//std::vector<Transform *> deserializeSceneGraph(char *data) {
//	std::vector<Transform *> nodes;
//	//char * currNode = data;
//	while (*data) {
//		Transform 
//		//unsigned size = 0;
//		//memCopy of node id + transform mat
//		memcpy(&slicePointer, data, sizeof(unsigned int)+ sizeof(glm::mat4));
//		slicePointer += sizeof(unsigned int) + sizeof(glm::mat4);
//		data += sizeof(unsigned int) + sizeof(glm::mat4);
//		//memcopy model ids size
//		memcpy(&slicePointer, data, sizeof(unsigned int));
//
//	}
//
//	return serializedNodes;
//}