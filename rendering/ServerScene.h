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
	void initialize_objects();
	void update();
	void render();
	void handlePlayerMovement(glm::vec3 destination);
	unsigned int serializeSceneGraph(char* data);
	unsigned int serializeSceneGraph(Transform* t, char* data);

private:
	const char* window_title = "CSE 125 Group 3";
	Shader * shader;
	Camera * camera;

	Cube * cube;
	Player * player;
	Model * player_m;
	Transform * root;
	Transform * player_t;

	std::vector<ModelData> models;

	double time = 0.0;
};

#endif
