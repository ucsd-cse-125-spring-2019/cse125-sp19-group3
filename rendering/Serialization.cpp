#include "Serialization.h"


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
void Serialization::deserializeLeaderBoard(char* lb_data, LeaderBoard* leaderBoard)
{

	for (int i = 0; i < GAME_SIZE; i++)		// kills
	{
		memcpy(&leaderBoard->currentKills[i], lb_data, sizeof(int));
		lb_data += sizeof(int);
	}
	for (int i = 0; i < GAME_SIZE; i++)		// points
	{
		memcpy(&leaderBoard->currPoints[i], lb_data, sizeof(int));
		lb_data += sizeof(int);
	}
	for (int i = 0; i < GAME_SIZE; i++)		// prizes
	{
		memcpy(&leaderBoard->prizes[i], lb_data, sizeof(int));
		lb_data += sizeof(int);
	}

	memcpy(&leaderBoard->prizeChange, lb_data, sizeof(float));
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


Transform * Serialization::deserializeSceneGraph(char *data, unordered_map<unsigned int, Transform *> &clientSceneGraphMap) {
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
			size = node->deserializeAndUpdate(data);
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
			size = newNode->deserializeAndUpdate(data);
			clientSceneGraphMap.insert({ node_id, newNode });
			if (!root) {
				root = newNode;
			}
		}
		data += size;
	}
	return root;
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