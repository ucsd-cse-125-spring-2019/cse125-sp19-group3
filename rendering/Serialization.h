#pragma once

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
#include "ServerScene.h"

struct Serialization
{
public:
	
	static unsigned int serializeSceneGraph(Transform * node, char *data, unordered_map<unsigned int, Transform *> &serverSceneGraphMap);
	static unsigned int serializeAnimationMode(unordered_map<unsigned int, ScenePlayer> &scenePlayers, char *data);
	static unsigned int deserializeSingleNodeId(char *data);
	static Transform * deserializeSceneGraph(char *data, unordered_map<unsigned int, Transform *> &clientSceneGraphMap);
	static char * deserializeAnimationMode(char *data, vector<pair<unsigned int, vector<int>>> &animationModes);

};

