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
		// always set animationMode back to -1
		p.second.animationMode = -1;
	}
	return size;
}

// deserialize one of the leaderbo// serialize leaderboard
unsigned int Serialization::serializeLeaderBoard(char* lb_data, LeaderBoard* leaderBoard, 
	unordered_map<unsigned int, PlayerMetadata*>* playerMetadatas)
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
	for (int i = 0; i < GAME_SIZE; i++)			// killstreak
	{
		memcpy(lb_data, &leaderBoard->killStreaks[i], sizeof(int));
		size += sizeof(int);
		lb_data += sizeof(int);
	}

	for (int i = 0; i < GAME_SIZE; i++)			// deaths
	{
		memcpy(lb_data, &leaderBoard->currentDeaths[i], sizeof(int));
		size += sizeof(int);
		lb_data += sizeof(int);
	}
	for (int i = 0; i < GAME_SIZE; i++)			// global kills
	{
		memcpy(lb_data, &leaderBoard->globalKills[i], sizeof(int));
		size += sizeof(int);
		lb_data += sizeof(int);
	}
	
	memcpy(lb_data, &leaderBoard->deaths_this_tick, sizeof(int));
	size    += sizeof(int);
	lb_data += sizeof(int);

	// serialize who killed who (first int killers id, second int dead players id)
	// for every death this tick you should pop two elements off the front of the list (killer, dead_player)
	for (int i = 0; i < leaderBoard->deaths_this_tick; i++)
	{
		// get killer id
		int killer_id = leaderBoard->kill_map.front();
		leaderBoard->kill_map.pop_front();

		// get dead players id
		int dead_id = leaderBoard->kill_map.front();
		leaderBoard->kill_map.pop_front();

		// serialize killers id first
		memcpy(lb_data, &killer_id, sizeof(int));
		size	+= sizeof(int);
		lb_data += sizeof(int);

		// serialize dead players id second
		memcpy(lb_data, &dead_id, sizeof(int));
		size	+= sizeof(int);
		lb_data += sizeof(int);
	}

	// serialize total kill streaks
	memcpy(lb_data, &leaderBoard->total_killstreaks, sizeof(int));
	size += sizeof(int);
	lb_data += sizeof(int);

	// for each killstreak... serialize client ID followed by running number of kills
	for (int i = 0; i < leaderBoard->total_killstreaks; i++)
	{

		// TODO: REMOVE ME
		//int curr_client = leaderBoard->curr_killstreaks.front();

		// player id
		memcpy(lb_data, &leaderBoard->curr_killstreaks.front(), sizeof(int));
		leaderBoard->curr_killstreaks.pop_front();
		size += sizeof(int);
		lb_data += sizeof(int);

		// TODO: REMOVE ME
		//int ks = leaderBoard->curr_killstreaks.front();

		// num running kills
		memcpy(lb_data, &leaderBoard->curr_killstreaks.front(), sizeof(int));
		leaderBoard->curr_killstreaks.pop_front();
		size += sizeof(int);
		lb_data += sizeof(int);

		// TODO : REMOVE ME
		//logger()->debug("CLIENT {} KILLSTREAK {}", curr_client, ks);
	}

	// serialzie total shutdowns
	memcpy(lb_data, &leaderBoard->total_shutdowns, sizeof(int));
	size += sizeof(int);
	lb_data += sizeof(int);

	// for each shutdown... serialize killer id followed by dead player id
	for (int i = 0; i < leaderBoard->total_shutdowns; i++)
	{
		// killer id
		memcpy(lb_data, &leaderBoard->curr_shutdowns.front(), sizeof(int));
		leaderBoard->curr_shutdowns.pop_front();
		size += sizeof(int);
		lb_data += sizeof(int);

		// dead player id
		memcpy(lb_data, &leaderBoard->curr_shutdowns.front(), sizeof(int));
		leaderBoard->curr_shutdowns.pop_front();
		size += sizeof(int);
		lb_data += sizeof(int);
	}


	// reset values based per server tick
	leaderBoard->total_shutdowns = 0;
	leaderBoard->deaths_this_tick = 0;	
	leaderBoard->total_killstreaks = 0;

	return size;
}

// deserialize leaderboard
unsigned int Serialization::deserializeLeaderBoard(char* lb_data, LeaderBoard* leaderBoard, list<int>* killstreak_data)
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
	for (int i = 0; i < GAME_SIZE; i++)		// killstreak
	{
		memcpy(&leaderBoard->killStreaks[i], lb_data, sizeof(int));
		lb_data += sizeof(int);
		sz += sizeof(int);
	}
	for (int i = 0; i < GAME_SIZE; i++)		// deathcount
	{
		memcpy(&leaderBoard->currentDeaths[i], lb_data, sizeof(int));
		lb_data += sizeof(int);
		sz += sizeof(int);
	}
	for (int i = 0; i < GAME_SIZE; i++)		// glboal kills
	{
		memcpy(&leaderBoard->globalKills[i], lb_data, sizeof(int));
		lb_data += sizeof(int);
		sz += sizeof(int);
	}

	memcpy(&leaderBoard->deaths_this_tick, lb_data, sizeof(int));
	lb_data += sizeof(int);
	sz		+= sizeof(int);


	// deserialize who killed who (first int killers id, second int dead players id)
	// for every death this tick you should pop two elements off the front of the list (killer, dead_player)
	for (int i = 0; i < leaderBoard->deaths_this_tick; i++)
	{

		// get killer id & dead player ID's
		int killer_id = -1;
		memcpy(&killer_id, lb_data, sizeof(int));
		lb_data += sizeof(int);
		sz		+= sizeof(int);

		int dead_id = -1;
		memcpy(&dead_id, lb_data, sizeof(int));
		lb_data += sizeof(int);
		sz		+= sizeof(int);

		// add both to kill map (killer id then dead player id)
		leaderBoard->kill_map.push_back(killer_id);
		leaderBoard->kill_map.push_back(dead_id);

	}

	// deserialize total killstreaks
	int total_killstreaks = 0;
	memcpy(&total_killstreaks, lb_data, sizeof(int));
	lb_data += sizeof(int);
	sz += sizeof(int);

	// deserialize player id then total kills & add to killstreak_data list
	for (int i = 0; i < total_killstreaks; i++)
	{
		int curr_client_id = -1;
		int curr_kills = -1;

		memcpy(&curr_client_id, lb_data, sizeof(int));
		lb_data += sizeof(int);
		sz		+= sizeof(int);

		memcpy(&curr_kills, lb_data, sizeof(int));
		lb_data += sizeof(int);
		sz		+= sizeof(int);

		leaderBoard->curr_killstreaks.push_back(curr_client_id);
		leaderBoard->curr_killstreaks.push_back(curr_kills);
	}

	// deserialize total shutdowns
	int total_shutdowns = 0;
	memcpy(&total_shutdowns, lb_data, sizeof(int));
	lb_data += sizeof(int);
	sz += sizeof(int);

	// deserialize killer id then dead player id
	for (int i = 0; i < total_shutdowns; i++)
	{
		int killer_id = -1;
		int dead_id = -1;

		memcpy(&killer_id, lb_data, sizeof(int));
		lb_data += sizeof(int);
		sz += sizeof(int);

		memcpy(&dead_id, lb_data, sizeof(int));
		lb_data += sizeof(int);
		sz += sizeof(int);

		leaderBoard->curr_shutdowns.push_back(killer_id);
		leaderBoard->curr_shutdowns.push_back(dead_id);
	}

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
			size = node->deserializeAndUpdate(data, particleShader, particleTexture,false);
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
			size = newNode->deserializeAndUpdate(data, particleShader, particleTexture,true);
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