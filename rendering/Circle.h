#include <GL/glew.h>
#include <glm/glm.hpp>
#include "shader.h"
#include "../networking/KillStreak/CoreTypes.hpp"

using namespace std;

class Circle
{
public:
	vector<glm::vec3> vertices;
	vector<unsigned int> indices;
	glm::mat4 localMtx = glm::mat4(1.0f);
	unsigned int VAO, VBO, EBO;
	float radius = 10.0f;

	/*  Functions   */
	// constructor, expects a filepath to a 3D model.
	Circle() { 
		createCircle(radius); 
		setup();
	}

	// draws the model, and thus all its meshes
	void draw(Shader * shader, const glm::mat4 &parentMtx, const glm::mat4 &viewProjMtx, unsigned int frameBuffer) {
		glm::mat4 modelMtx = parentMtx * localMtx;
		shader->use();
		shader->setMat4("ModelViewProjMtx", viewProjMtx * modelMtx);

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void createCircle(float radius) {
		vertices.push_back(glm::vec3(0));
		for (unsigned int i = 0; i < 100; i++) {
			float theta = 2.0f * glm::pi<float>() * float(i) / 100;
			float x = radius * glm::cos(theta);
			float y = radius * glm::sin(theta);
			vertices.push_back(glm::vec3(x, 0, y));
		}
		vertices.push_back(vertices[1]);
		for (unsigned int i = 1; i < 101; i++) {
			indices.push_back(0);
			indices.push_back(i);
			indices.push_back(i + 1);
		}
	}

	void setup() {
		// create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		// bind vertices array
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		// bind indices array
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	}
};