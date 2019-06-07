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
	void initialize_objects(ClientGame * game, ClientNetwork* network, LeaderBoard* leaderBoard, list<int>* killstreak_data);
	void initialize_skills(ArcheType selected_type);
	void playPreparePhaseBGM();
	void playKillPhaseBGM();
	void playFinalRoundBGM();
	void playCountdown();
	void playButtonPress();
	void playInvalidButtonPress();
	void playChaching();
	void playInvest();
	void playRoundOver();
	void playKillStreak();
	void playShutdown();
	void playVictory();
	void playTimeup();
	//void playerInit(const ScenePlayer &player);
	void clean_up();
	GLFWwindow * create_window();
	void resize_callback(GLFWwindow* window, int width, int height);
	void idle_callback();
	void text_input(GLFWwindow *win, unsigned int codepoint);
	void display_callback(GLFWwindow* window);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	glm::vec3 viewToWorldCoordTransform(int mouse_x, int mouse_y);
	int handleInitScenePacket(char * data);
	void handleServerTickPacket(char* data);
	void setRoot(Transform * newRoot);
	void updateTimers(nanoseconds timePassed);
	void renderPreparePhase(GLFWwindow* window);
	void renderKillPhase(GLFWwindow* window);
	void renderLobbyPhase(GLFWwindow* window);
	void renderFinalPhase(GLFWwindow* window);
	void renderSummaryPhase(GLFWwindow* window);
	void initialize_UI(GLFWwindow* window);
	void resetPreKillPhase();
	int getPlayerGold();
	void updatePlayerGold(int curr_gold);
	vector<Skill> getPlayerSkills();
	vector<string> getUsernames();
	bool checkInAnimation();
	void resetGUIStatus();
	vector<int> getInvestmentInfo();
	void clearInvestmentInfo();

private:
	Audio audio;
	float min_scroll = 20.0f;
	float max_scroll = 60.0f;
	const char* window_title = "CSE 125 Group 3";
	Shader * animationShader, * staticShader, * particleShader, * circleShader;
	Model * floor;
	Model * arrow;
	Model * cross;
	Model * circle;
	Circle * range;
	ScenePlayer player;
	Transform * root;
	LeaderBoard* leaderBoard;
	list<int>* killstreak_data;
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
	nanoseconds kingSilenceHemisphereTimer;
	bool isCharging = false;

	// void removeTransform(Transform * parent, const unsigned int node_id);
	
};

class Window_static
{
public:
	static ClientScene * scene;
	static void initialize_objects(ClientGame * game, ClientNetwork * network, LeaderBoard* leaderBoard, list<int>* killstread_data) { scene->initialize_objects(game, network, leaderBoard, killstread_data); };
	static void initialize_skills(ArcheType selected_type) { scene->initialize_skills(selected_type); };
	static void updateTimers(nanoseconds timePassed) { scene->updateTimers(timePassed); };
	static void initialize_UI(GLFWwindow* window) { scene->initialize_UI(window); };
	static void clean_up() { scene->clean_up(); };
	static GLFWwindow * create_window() { return scene->create_window(); };
	static void resize_callback(GLFWwindow* win, int width, int height) {
		scene->resize_callback(win, width, height);
	};
	static void text_callback(GLFWwindow *win, unsigned int codepoint) { scene->text_input(win, codepoint); };
	static void idle_callback() { scene->idle_callback(); };
	static void display_callback(GLFWwindow* win) { scene->display_callback(win); };
	static void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) { scene->key_callback(win, key, scancode, action, mods); };
	static void scroll_callback(GLFWwindow* win, double xoffset, double yoffset) { scene->scroll_callback(win, xoffset, yoffset); };
	static void mouse_button_callback(GLFWwindow* win, int button, int action, int mods) { scene->mouse_button_callback(win, button, action, mods); };
	static int handleInitScenePacket(char * data) { scene->handleInitScenePacket(data); };
	static void handleServerTickPacket(char* data) { scene->handleServerTickPacket(data); };
	static void resetGUIStatus() { scene->resetGUIStatus(); };

	static void resetPreKillPhase() { scene->resetPreKillPhase(); };
	static void playPreparePhaseBGM() { scene->playPreparePhaseBGM(); };
	static void playKillPhaseBGM() { scene->playKillPhaseBGM(); };
	static void playFinalRoundBGM() { scene->playFinalRoundBGM(); };
	static void playRoundOver() { scene->playRoundOver(); };
	static void playCountdown() { scene->playCountdown(); };
	static void playButtonPress() { scene->playButtonPress(); };
	static void playChaching() { scene->playChaching(); };
	static void playInvest() { scene->playInvest(); };
	static void playInvalidButtonPress() { scene->playInvalidButtonPress(); };
	static void playKillStreak() { scene->playKillStreak(); };
	static void playShutdown() { scene->playShutdown(); };
	static void playVictory() { scene->playVictory(); };
	static void playTimeup() { scene->playTimeup(); };

	static int getPlayerGold() { return scene->getPlayerGold(); };
	static vector<Skill> getPlayerSkills() { return scene->getPlayerSkills(); };
	static vector<string> getUsernames() { return scene->getUsernames(); };
	static vector<int> getInvestmentInfo() { return scene->getInvestmentInfo(); };
	static void clearInvestmentInfo() { return scene->clearInvestmentInfo(); };
	static void updatePlayerGold(int curr_gold) { return scene->updatePlayerGold(curr_gold); };

};


