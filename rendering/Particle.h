#ifndef _PARTICLE_
#define _PARTICLE_

#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include "glm/ext.hpp"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <math.h>
#include <cstdint>
#include <algorithm>
#include <iomanip>
#include <random>
#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stb_image.h"

struct Particle {
	glm::vec3 pos, speed;
	unsigned char r, g, b, a; // Color
	float size;
	float life; // Remaining life of the particle. if < 0 : dead and unused.
	float cameradistance;

	bool operator<(Particle& that) {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

class Particles {

public:
	Shader * shader;
	glm::vec3 translation;
	Particles(GLuint, Shader *, glm::vec3);
	~Particles();
	void update(glm::vec3 move);
	/// better be called before swapping buffer
	void draw();
	void reinitParticle(Particle&);
	GLuint VertexArrayID, Texture;
	GLuint billboard_vertex_buffer;
	GLuint particles_position_buffer;
	GLuint particles_color_buffer;
	// controls
};

#endif