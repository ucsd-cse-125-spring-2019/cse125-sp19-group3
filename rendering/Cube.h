#ifndef _CUBE_H_
#define _CUBE_H_

#define GLFW_INCLUDE_GLEXT
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// Use of degrees is deprecated. Use radians instead.
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Cube
{
public:
	Cube();
	~Cube();

	glm::mat4 toWorld;

	void draw(GLuint);
	void update();
	void spin(float);

	// These variables are needed for the shader program
	GLuint VBO, VBO2, VAO, EBO;
	GLuint uModel, uModelViewProjection;
};

// Define the coordinates and indices needed to draw the cube. Note that it is not necessary
// to use a 2-dimensional array, since the layout in memory is the same as a 1-dimensional array.
// This just looks nicer since it's easy to tell what coordinates/indices belong where.
const GLfloat vertices[24][3] = {
	// "Front" vertices
	{-2.0, -2.0,  2.0}, {2.0, -2.0,  2.0}, {2.0,  2.0,  2.0}, {-2.0,  2.0,  2.0},
	// "Back" vertices
	{-2.0, -2.0, -2.0}, {2.0, -2.0, -2.0}, {2.0,  2.0, -2.0}, {-2.0,  2.0, -2.0},
	// Top
	{-2.0, 2.0,  2.0}, {2.0, 2.0,  2.0}, {2.0,  2.0,  -2.0}, {-2.0,  2.0,  -2.0},
	// Bottom
	{-2.0, -2.0,  -2.0}, {2.0, -2.0,  -2.0}, {2.0,  -2.0,  2.0}, {-2.0,  -2.0,  2.0},
	// Left
	{-2.0, -2.0,  -2.0}, {-2.0, -2.0,  2.0}, {-2.0,  2.0,  2.0}, {-2.0,  2.0,  -2.0},
	// Right
	{2.0, -2.0,  2.0}, {2.0, -2.0,  -2.0}, {2.0,  2.0,  -2.0}, {2.0,  2.0,  2.0}
};

const GLfloat normals[24][3] = {
	// "Front" vertices
	{0,0,1}, {0,0,1}, {0,0,1}, {0,0,1},
	{0,0,-1}, {0,0,-1}, {0,0,-1}, {0,0,-1},
	{0,1,0}, {0,1,0}, {0,1,0}, {0,1,0},
	{0,-1,0}, {0,-1,0}, {0,-1,0}, {0,-1,0},
	{-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0},
	{1,0,0}, {1,0,0}, {1,0,0}, {1,0,0}
};

// Note that GL_QUADS is deprecated in modern OpenGL (and removed from OSX systems).
// This is why we need to draw each face as 2 triangles instead of 1 quadrilateral
const GLuint indices[6][6] = {
	// Front face
	{0, 1, 2, 0, 2, 3},
	// Back face
	{4, 5, 6, 4, 6, 7},
	// Top face
	{8, 9, 10, 8, 10, 11},
	// Bottom face
	{12, 13, 14, 12, 14, 15},
	// Left face
	{16, 17, 18, 16, 18, 19},
	// Right face
	{20, 21, 22, 20, 22, 23}
};

#endif

