#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Cube.h"
#include "shader.h"
#include "Model.h"
#include "Camera.h"
#include "Transform.h"
#include "Player.h"
#include "../networking/KillStreak/ClientGame.h"
#include "../networking/KillStreak/CoreTypes.hpp"

// On some systems you need to change this to the absolute path
#define VERTEX_SHADER_PATH "../shader.vert"
#define FRAGMENT_SHADER_PATH "../shader.frag"

class ClientScene {
public:
	int width;
	int height;
	void initialize_objects(ClientGame * game);
	void playerInit(const Player &player);
	void clean_up();
	GLFWwindow * create_window(int width, int height);
	void resize_callback(GLFWwindow* window, int width, int height);
	void idle_callback();
	void display_callback(GLFWwindow*);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	glm::vec3 viewToWorldCoordTransform(int mouse_x, int mouse_y);
	char* deserializeInitScene(char * data, unsigned int size);
	char* deserializeSceneGraph(char* data, unsigned int size);
	char* deserializeSceneGraph(Transform* t, char* data, unsigned int size);

private:
	const char* window_title = "CSE 125 Group 3";
	Shader * shader;
	Camera * camera;

	Cube * cube;
	Player player;
	Model * player_m;
	Transform * root;
	Transform * player_t;

	std::vector<ModelData> models;
	std::unordered_set<unsigned int> updated_ids;

	double time = 0.0;

	ClientGame * game;

	void removeTransform(Transform * parent, const unsigned int node_id);
};

class Window_static
{
public:
	static ClientScene * scene;
	static void initialize_objects(ClientGame * game) { scene->initialize_objects(game); };
	static void clean_up() { scene->clean_up(); };
	static GLFWwindow * create_window(int width, int height) { return scene->create_window(width, height); };
	static void resize_callback(GLFWwindow* win, int width, int height) {
		scene->resize_callback(win, width, height);
	};
	static void idle_callback() { scene->idle_callback(); };
	static void display_callback(GLFWwindow* win) { scene->display_callback(win); };
	static void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) { scene->key_callback(win, key, scancode, action, mods); };
	static void scroll_callback(GLFWwindow* win, double xoffset, double yoffset) { scene->scroll_callback(win, xoffset, yoffset); };
	static void mouse_button_callback(GLFWwindow* win, int button, int action, int mods) { scene->mouse_button_callback(win, button, action, mods); };
};


