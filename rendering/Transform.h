#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_set>
#include <unordered_map>
#include "Model.h"
#include "Shader.h"
#include "Particle.h"

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
	bool isEvading = false;
	bool isInvincible = false;
	bool isCharging = false;
	bool isInvisible = false;
	float initialRotation;
	glm::mat4 M;
	glm::mat4 translation;
	glm::mat4 rotation;
	glm::mat4 scale;
	std::unordered_set<unsigned int> children_ids;
	std::unordered_set<unsigned int> model_ids;
	glm::vec3 destination;
	float speed;
	glm::vec3 direction;
	Particles * particle_effect;
	Transform();
	
	Transform(unsigned int nodeId, glm::mat4 M);
	Transform(unsigned int nodeId, glm::mat4 translation, glm::mat4 rotation, glm::mat4 scale);
	void Transform::setDestination(glm::mat4 & updatedM);
	void addChild(const unsigned int id);
	void removeChild(unsigned int id);
	unsigned int serialize(char * data);
	unsigned deserializeAndUpdate(char * data, Shader* particleShader, GLuint particleTexture);
	void draw(std::unordered_map<unsigned int, ModelData> &models, const glm::mat4 &parentMtx, const glm::mat4 &viewProjMtx, unordered_map<unsigned int, Transform *> &sceneGraphMap);
	void update();
	void clientUpdate();
	bool isCollided(glm::vec3 forwardVector, unordered_map<unsigned int, float> &modelRadius, unordered_map<unsigned int, Transform *> &sceneGraphMap, Transform * otherNode, unordered_map<unsigned int, glm::vec3> &modelBoundingBoxes, bool toEnv);
};

#endif