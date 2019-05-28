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
	static unsigned int serializeLeaderBoard(char* leaderBuffPtr, LeaderBoard* leaderBoard);
	static unsigned int deserializeLeaderBoard(char* lb_data, LeaderBoard* leaderBoard);
	static unsigned int deserializeSingleNodeId(char *data);
	static Transform * deserializeSceneGraph(char *data, unordered_map<unsigned int, Transform *> &clientSceneGraphMap);
};

