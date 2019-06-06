#ifndef _MESH_H_
#define _MESH_H_

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "shader.h"

using namespace std;

#define NUM_BONES_PER_VERTEX 8

struct Vertex {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;

	int IDs[NUM_BONES_PER_VERTEX] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	float Weights[NUM_BONES_PER_VERTEX] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
};

struct Texture {
	unsigned int id;
	string type;
	string path;
};

struct BoneInfo {
	glm::mat4 BoneOffset;
	glm::mat4 FinalTransformation;
	string BoneLocation;
	string BoneName;
};

class Mesh {
public:
	/*  Mesh Data  */
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;

	/*  Functions  */
	// constructor
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		// now that we have all the required data, set the vertex buffers and its attribute pointers.
		setupMesh();
	}

	// render the mesh
	void draw(Shader * shader, const glm::mat4 &viewProjMtx, unsigned int textureId)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
	}

	void prepareBloomEffect(unsigned int width, unsigned int height) {
		unsigned int frameBufferTexture[2];

		glGenFramebuffers(1, &bloomFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
		glGenTextures(1, &frameBufferTexture[0]);
		glBindTexture(GL_TEXTURE_2D, frameBufferTexture[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture[0], 0);

		glGenFramebuffers(1, &normalFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, normalFBO);
		glGenTextures(1, &frameBufferTexture[1]);
		glBindTexture(GL_TEXTURE_2D, frameBufferTexture[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture[1], 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		prepareQuad();
	}

	void renderToBloomBuffer(Shader * shader, const glm::mat4 &viewProjMtx, unsigned int textureId) {
		glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
		glEnable(GL_DEPTH_TEST);
		draw(shader, viewProjMtx, textureId);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void renderToNormalBuffer(Shader * shader, const glm::mat4 &viewProjMtx, unsigned int textureId) {
		glBindFramebuffer(GL_FRAMEBUFFER, normalFBO);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
		glEnable(GL_DEPTH_TEST);
		draw(shader, viewProjMtx, textureId);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void blendBloomTexture(unsigned int width, unsigned int height, Shader * blurShader, Shader * blendShader) {
		unsigned int pingpongFBO[2];
		unsigned int pingpongBuffer[2];
		glGenFramebuffers(2, pingpongFBO);
		glGenTextures(2, pingpongBuffer);
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
			glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL
			);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
			);
		}

		// blurring
		bool horizontal = true, first_iteration = true;
		int amount = 10;
		blurShader->use();
		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			blurShader->setInt("horizontal", horizontal);
			glBindTexture(
				GL_TEXTURE_2D, first_iteration ? bloomFBO : pingpongBuffer[!horizontal]
			);
			renderQuad();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		blendShader->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bloomFBO);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
		blendShader->setFloat("exposure", 1.0f);
		renderQuad();
	}

	void blendBloomEffect(Shader * blendShader) {

	}

private:
	/*  Render data  */
	unsigned int VBO, EBO;
	unsigned int bloomFBO, normalFBO;

	unsigned int quadVAO, quadVBO, quadVBO2, quadEBO;

	vector<glm::vec2> quadVertices {
		{-1.0f, -1.0f},
		{1.0f, -1.0f},
		{1.0f, 1.0f},
		{-1.0f, 1.0f}
	};

	vector<glm::vec2> quadTexCoords{
		{0, 0},
		{1, 0},
		{1, 1},
		{0, 1}
	};

	vector<unsigned int> quadIndices {
		0, 1, 2,
		0, 2, 3
	};

	/*  Functions    */
	// initializes all the buffer objects/arrays
	void setupMesh()
	{
		// create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
		// bone indices
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, IDs));	
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (void*)(offsetof(Vertex, IDs) + 4 * sizeof(unsigned int)));
		// weights
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Weights));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, Weights) + 4 * sizeof(float)));

		glBindVertexArray(0);
	}

	void prepareQuad() {
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glGenBuffers(1, &quadEBO);

		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(glm::vec2), &quadVertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, quadVBO2);
		glBufferData(GL_ARRAY_BUFFER, quadTexCoords.size() * sizeof(glm::vec2), &quadTexCoords[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, quadIndices.size() * sizeof(unsigned int), &quadIndices[0], GL_STATIC_DRAW);
	
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

		glBindVertexArray(0);
	}

	void renderQuad() {
		glBindVertexArray(quadVAO);
		glDrawElements(GL_TRIANGLES, quadIndices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
};
#endif