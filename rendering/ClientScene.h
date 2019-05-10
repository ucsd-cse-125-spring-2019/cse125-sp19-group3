#include <iostream>
#include <vector>
#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "Cube.h"
#include "shader.h"
#include "Model.h"
#include "Camera.h"
#include "Transform.h"
#include "ScenePlayer.h"
#include "../networking/KillStreak/ClientGame.h"
#include "../networking/KillStreak/CoreTypes.hpp"
#include "../rendering/Serialization.h"

// On some systems you need to change this to the absolute path
#define VERTEX_SHADER_PATH "../animatedShader.vert"
#define FRAGMENT_SHADER_PATH "../animatedShader.frag"

#define TOON_VERTEX_SHADER_PATH "../toonshader.vert"
#define TOON_FRAGMENT_SHADER_PATH "../toonshader.frag"

class ClientScene {
public:
	int width;
	int height;

	std::unordered_map<unsigned int, Transform *> clientSceneGraphMap;

	void initialize_objects(ClientGame * game);
	void playerInit(const ScenePlayer &player);
	void clean_up();
	GLFWwindow * create_window(int width, int height);
	void resize_callback(GLFWwindow* window, int width, int height);
	void idle_callback();
	void display_callback(GLFWwindow*);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	glm::vec3 viewToWorldCoordTransform(int mouse_x, int mouse_y);
	void handleInitScenePacket(char * data);
	void handleServerTickPacket(char* data);
	void setRoot(Transform * newRoot);

private:
	const char* window_title = "CSE 125 Group 3";
	Shader * animationShader, * staticShader;
	Camera * camera;

	Cube * cube;
	ScenePlayer player;
	Transform * root;

	std::unordered_map<unsigned int, ModelData> models;
	std::unordered_set<unsigned int> updated_ids;

	double time = 0.0;

	ClientGame * game;
	vector<Transform *> env_objs;

	// void removeTransform(Transform * parent, const unsigned int node_id);
	
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
	static void handleInitScenePacket(char * data) { scene->handleInitScenePacket(data); };
	static void handleServerTickPacket(char* data) { scene->handleServerTickPacket(data); };
};


