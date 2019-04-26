#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <iostream>
#include <vector>

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include "Cube.h"
#include "shader.h"
#include "Model.h"
#include "Camera.h"
#include "Transform.h"
#include "Player.h"

// On some systems you need to change this to the absolute path
#define VERTEX_SHADER_PATH "../shader.vert"
#define FRAGMENT_SHADER_PATH "../shader.frag"

class Window {
public:
	int width;
	int height;
	void initialize_objects();
	void clean_up();
	GLFWwindow * create_window(int width, int height);
	void resize_callback(GLFWwindow* window, int width, int height);
	void idle_callback();
	void display_callback(GLFWwindow*);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	glm::vec3 viewToWorldCoordTransform(int mouse_x, int mouse_y);

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

class Window_static
{
public:
	static Window * window;
	static void initialize_objects() { window->initialize_objects(); };
	static void clean_up() { window->clean_up(); };
	static GLFWwindow * create_window(int width, int height) { return window->create_window(width, height); };
	static void resize_callback(GLFWwindow* win, int width, int height) {
		window->resize_callback(win, width, height);
	};
	static void idle_callback() { window->idle_callback(); };
	static void display_callback(GLFWwindow* win) { window->display_callback(win); };
	static void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) { window->key_callback(win, key, scancode, action, mods); };
	static void scroll_callback(GLFWwindow* win, double xoffset, double yoffset) { window->scroll_callback(win, xoffset, yoffset); };
	static void mouse_button_callback(GLFWwindow* win, int button, int action, int mods) { window->mouse_button_callback(win, button, action, mods); };
};


