#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <iostream>
#include <vector>
#include <GL/glew.h>


#include "Cube.h"
#include "shader.h"
#include "Model.h"
#include "Camera.h"
#include "Transform.h"
#include "Player.h"

// On some systems you need to change this to the absolute path
#define VERTEX_SHADER_PATH "../shader.vert"
#define FRAGMENT_SHADER_PATH "../shader.frag"

class ServerScene {
public:
	int width;
	int height;

	std::map<unsigned int, Player *> playerMap;

	ServerScene();
	~ServerScene();
	void addPlayer(unsigned int playerId);
	void update();
	void handlePlayerMovement(unsigned int player_id, glm::vec3 destination);
	unsigned int serializeInitScene(char* data, unsigned int playerId, unsigned int playerRootId);
	unsigned int serializeSceneGraph(char* data);
	unsigned int serializeSceneGraph(Transform* t, char* data);

private:
	Player * player;
	Transform * root;
	Transform * playerRoot;

	double time = 0.0;
	unsigned int playerIdCounter = 0;
	unsigned int nodeIdCounter = 0;
};

#endif
