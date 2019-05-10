#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_set>
#include <unordered_map>
#include "Model.h"
#include "Shader.h"

enum RenderMode {
	COLOR,
	TEXTURE
};

struct ModelData {
	Model * model;
	glm::vec4 color;
	Shader * shader;
	RenderMode renderMode;
	GLuint texID;
};

enum ModelIds {
	PLAYER = 0,
	QUAD = 1,
	GUN = 2,
	BAR = 3,
	HEAD = 4
};

class Transform
{
public:
	unsigned int node_id;
	bool enabled = true;

	glm::mat4 M;
	glm::mat4 translation;
	glm::mat4 rotation;
	glm::mat4 scale;
	std::unordered_set<unsigned int> children_ids;
	std::unordered_set<unsigned int> model_ids;

	
	Transform();
	Transform(char * data);
	Transform(unsigned int nodeId, glm::mat4 M);
	Transform(unsigned int nodeId, glm::mat4 translation, glm::mat4 rotation, glm::mat4 scale);

	void addChild(const unsigned int id);
	void removeChild(unsigned int id);
	unsigned int serialize(char * data);
	unsigned deserializeAndUpdate(char * data);
	void draw(std::unordered_map<unsigned int, ModelData> &models, const glm::mat4 &parentMtx, const glm::mat4 &viewProjMtx, unordered_map<unsigned int, Transform *> &sceneGraphMap);
	void update();
};

#endif