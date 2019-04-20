#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_set>
#include <unordered_map>
#include "Model.h"
#include "Shader.h"

class Transform
{
public:
	bool enabled = true;

	glm::mat4 M;
	std::unordered_map<unsigned int, Transform*> children;
	std::unordered_set<unsigned int> model_ids;

	Transform();
	Transform(glm::mat4 C);

	void addChild(const unsigned int id, Transform* child);
	void removeChild(unsigned int id);

	void draw(GLuint shaderProgram, const glm::mat4 &parentMtx, const glm::mat4 &viewProjMtx);
};

#endif