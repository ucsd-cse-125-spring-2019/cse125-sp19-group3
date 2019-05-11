#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <queue> 

#include "Cube.h"
#include "shader.h"
#include "Model.h"
#include "Camera.h"
#include "Transform.h"
#include "ScenePlayer.h"
#include "../networking/KillStreak/PlayerData.hpp"

// On some systems you need to change this to the absolute path
#define VERTEX_SHADER_PATH "../shader.vert"
#define FRAGMENT_SHADER_PATH "../shader.frag"

class ServerScene {
public:
	int width;
	int height;

	unordered_map<unsigned int, ScenePlayer> scenePlayers;
	unordered_map<unsigned int, float> model_radius;
	std::vector<Transform *> env_objs;
	ServerScene();
	~ServerScene();
	void addPlayer(unsigned int playerId, ArcheType modelType);
	void update();
	void handlePlayerMovement(unsigned int player_id, glm::vec3 destination);
	void checkAndHandleCollision(unsigned int playerId);
	/*unsigned int serializeInitScene(char* data, unsigned int playerId, unsigned int playerRootId);
	unsigned int serializeSceneGraph(char* data);
	std::pair<char *, unsigned int> serializeSceneGraph(Transform* t, char* data);*/
	Transform * getRoot();
	unordered_map<unsigned int, Transform *> serverSceneGraphMap;
	void initEnv();
	void initModelRadius();

private:
	Transform * root;
	Transform * playerRoot;
	Transform * skillRoot;

	double time = 0.0;
	unsigned int nodeIdCounter = 0;
};

#endif
