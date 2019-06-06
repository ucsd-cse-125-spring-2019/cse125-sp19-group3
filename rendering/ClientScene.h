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
#include "Particle.h"
#include "Circle.h"
#include "Audio.h"
#include "../networking/KillStreak/ClientGame.h"
#include "../networking/KillStreak/ClientNetwork.hpp"
#include "../networking/KillStreak/CoreTypes.hpp"
#include "../rendering/Serialization.h"
// On some systems you need to change this to the absolute path
#define VERTEX_SHADER_PATH "../shaders/animatedShader.vert"
#define FRAGMENT_SHADER_PATH "../shaders/animatedShader.frag"

#define TOON_VERTEX_SHADER_PATH "../shaders/toonshader.vert"
#define TOON_FRAGMENT_SHADER_PATH "../shaders/toonshader.frag"

#define PARTICLE_VERTEX_SHADER_PATH "../shaders/particleShader.vert"
#define PARTICLE_FRAGMENT_SHADER_PATH "../shaders/particleShader.frag"

#define CIRCLE_VERTEX_SHADER_PATH "../shaders/circleshader.vert"
#define CIRCLE_FRAGMENT_SHADER_PATH "../shaders/circleshader.frag"

#define BLUR_VERTEX_SHADER_PATH "../shaders/blurShader.vert"
#define BLUR_FRAGMENT_SHADER_PATH "../shaders/blurShader.frag"

#define BLEND_VERTEX_SHADER_PATH "../shaders/blendShader.vert"
#define BLEND_FRAGMENT_SHADER_PATH "../shaders/blendShader.frag"

#define MAX_KILL_UPDATES 3

class ClientScene {
public:
	Camera * camera;
	int width;
	int height;
	std::unordered_map<unsigned int, Transform *> clientSceneGraphMap;
	vector<ArcheType> archetypes;	// list of all player archetypes
	vector<string> usernames;		// list of all player usernames ordered by index of client id on server

	GLuint particleTexture;
	void initialize_objects(ClientGame * game, ClientNetwork* network, LeaderBoard* leaderBoard);
	void initialize_skills(ArcheType selected_type);
	void blendBloomEffect();
	//void playerInit(const ScenePlayer &player);
	void clean_up();
	GLFWwindow * create_window(int width, int height);
	void resize_callback(GLFWwindow* window, int width, int height);
	void idle_callback();
	void text_input(GLFWwindow *win, unsigned int codepoint);
	void display_callback(GLFWwindow* window);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	glm::vec3 viewToWorldCoordTransform(int mouse_x, int mouse_y);
	void handleInitScenePacket(char * data);
	void handleServerTickPacket(char* data);
	void setRoot(Transform * newRoot);
	void updateTimers(nanoseconds timePassed);
	void renderPreparePhase(GLFWwindow* window);
	void renderKillPhase(GLFWwindow* window);
	void renderLobbyPhase(GLFWwindow* window);
	void initialize_UI(GLFWwindow* window);
	void resetPreKillPhase();
	int getPlayerGold();
	vector<Skill> getPlayerSkills();
	vector<string> getUsernames();
	bool checkInAnimation();
	void resetGUIStatus();
private:
	Audio audio;
	float min_scroll = 20.0f;
	float max_scroll = 60.0f;
	const char* window_title = "CSE 125 Group 3";
	Shader * animationShader, * staticShader, * particleShader, * circleShader;
	Shader * blurShader, * blendShader;
	Model * floor;
	Model * arrow;
	Model * cross;
	Model * quad;
	Circle * range;
	ScenePlayer player;
	Transform * root;
	LeaderBoard* leaderBoard;
	int killTextDeterminant = 0;
	std::unordered_map<unsigned int, ModelData> models;
	std::unordered_set<unsigned int> updated_ids;

	double time = 0.0;

	ClientGame * game;
	ClientNetwork * network;
	vector<Transform *> env_objs;
	//vector<Skill> personal_skills;
	vector<nanoseconds> skill_timers;
	nanoseconds respawn_timer;		// when should client respawn from death
	nanoseconds animation_timer;
	nanoseconds skillDurationTimer; // used for invisibility, silence
	nanoseconds evadeDurationTimer; // used for evade
	nanoseconds sprintDurationTimer; // used for sprint
	nanoseconds invincibilityTimer;
	bool isCharging = false;

	unsigned int bloomFBO, normalFBO;
	unsigned int quadVAO, quadVBO, quadVBO2, quadEBO;
	unsigned int frameBufferTexture[2];

	unsigned int textureId;

	glm::mat4 viewportM = glm::mat4(1.0f);

	vector<glm::vec3> quadVertices{
		{ -1.0f, -1.0f, 0.0f },
		{ 1.0f, -1.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ -1.0f, 1.0f, 0.0f }
	};

	vector<glm::vec2> quadTexCoords{
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f }
	};

	vector<unsigned int> quadIndices{
		0, 1, 2,
		0, 2, 3
	};

	void prepareBloomEffect() {
		glGenFramebuffers(1, &bloomFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
		glGenTextures(1, &frameBufferTexture[0]);
		glBindTexture(GL_TEXTURE_2D, frameBufferTexture[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture[1], 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		prepareQuad();
	}

	void prepareQuad() {
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glGenBuffers(1, &quadVBO2);
		glGenBuffers(1, &quadEBO);

		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(glm::vec3), &quadVertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, quadVBO2);
		glBufferData(GL_ARRAY_BUFFER, quadTexCoords.size() * sizeof(glm::vec2), &quadTexCoords[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, quadIndices.size() * sizeof(unsigned int), &quadIndices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

		glBindVertexArray(0);

		textureId = TextureFromFile("../textures/floor.png");

	}

	void renderQuad() {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glBindVertexArray(quadVAO);
		glDrawElements(GL_TRIANGLES, quadIndices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
	}
	
};

class Window_static
{
public:
	static ClientScene * scene;
	static void initialize_objects(ClientGame * game, ClientNetwork * network, LeaderBoard* leaderBoard) { scene->initialize_objects(game, network, leaderBoard); };
	static void initialize_skills(ArcheType selected_type) { scene->initialize_skills(selected_type); };
	static void updateTimers(nanoseconds timePassed) { scene->updateTimers(timePassed); };
	static void initialize_UI(GLFWwindow* window) { scene->initialize_UI(window); };
	static void clean_up() { scene->clean_up(); };
	static GLFWwindow * create_window(int width, int height) { return scene->create_window(width, height); };
	static void resize_callback(GLFWwindow* win, int width, int height) {
		scene->resize_callback(win, width, height);
	};
	static void text_callback(GLFWwindow *win, unsigned int codepoint) { scene->text_input(win, codepoint); };
	static void idle_callback() { scene->idle_callback(); };
	static void display_callback(GLFWwindow* win) { scene->display_callback(win); };
	static void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) { scene->key_callback(win, key, scancode, action, mods); };
	static void scroll_callback(GLFWwindow* win, double xoffset, double yoffset) { scene->scroll_callback(win, xoffset, yoffset); };
	static void mouse_button_callback(GLFWwindow* win, int button, int action, int mods) { scene->mouse_button_callback(win, button, action, mods); };
	static void handleInitScenePacket(char * data) { scene->handleInitScenePacket(data); };
	static void handleServerTickPacket(char* data) { scene->handleServerTickPacket(data); };
	static void resetGUIStatus() { scene->resetGUIStatus(); };

	static void resetPreKillPhase() { scene->resetPreKillPhase(); };
	static int getPlayerGold() { return scene->getPlayerGold(); };
	static vector<Skill> getPlayerSkills() { return scene->getPlayerSkills(); };
	static vector<string> getUsernames() { return scene->getUsernames(); };
};


