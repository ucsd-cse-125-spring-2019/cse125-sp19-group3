#include "ClientScene.h"
#include "nlohmann\json.hpp"
#include "GUILayout.h"
#include <fstream>

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#define EVADE_INDEX 0
#define PROJ_INDEX 1
#define OMNI_SKILL_INDEX 2
#define DIR_SKILL_INDEX 3
#define UNSILENCE 34
#define VISIBILITY 14

using json = nlohmann::json;
struct nk_context * ctx;
struct nk_colorf bg = { 0.1f,0.1f,0.1f,1.0f };
ClientScene * Window_static::scene = new ClientScene();
struct media media;

void ClientScene::initialize_objects(ClientGame * game, ClientNetwork * network)
{
	camera = new Camera();
	initCamPos = camera->cam_pos;
	camera->SetAspect(width / height);
	camera->Reset();

	animationShader = new Shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
	staticShader = new Shader(TOON_VERTEX_SHADER_PATH, TOON_FRAGMENT_SHADER_PATH);
	ifstream json_model_paths("../model_paths.json");
	json pathObjs = json::parse(json_model_paths);
	for (auto & obj : pathObjs["data"]) {
		if (obj["animated"]) {
			models[(unsigned int)obj["model_id"]] = ModelData{ 
				new Model(obj["path"], obj["texture_path"], true), 
				glm::vec4((float)(obj["color_rgb"][0]), (float)(obj["color_rgb"][1]), (float)(obj["color_rgb"][2]), 1.0f), 
				animationShader, 
				COLOR, 
				0 
			};
			for (unsigned int i = 0; i < 8; i++) {
				models[(unsigned int)obj["model_id"]].model->animation_frames.push_back(vector<float>{ ((unsigned int)obj["animations"][i][0]) / 30.0f, ((unsigned int)obj["animations"][i][1]) / 30.0f });
			}
		}
		else {
			models[(unsigned int)obj["model_id"]] = ModelData{ new Model(obj["path"], obj["texture_path"], false), glm::vec4((float)(obj["color_rgb"][0]), (float)(obj["color_rgb"][1]), (float)(obj["color_rgb"][2]), 1.0f), staticShader, COLOR, 0 };
		}
	}

	ifstream json_model_locations("../env_model_locations.json");
	unsigned int envCounter = 4000000000;
	json jsonObjs = json::parse(json_model_locations);
	for (auto & obj : jsonObjs["data"]) {
		Transform * envobj = new Transform(envCounter++, glm::translate(glm::mat4(1.0f), glm::vec3((float)(obj["translate"][0]), (float)(obj["translate"][1]), (float)(obj["translate"][2]))),
			glm::rotate(glm::mat4(1.0f), (float)obj["rotate"] / 180.0f * glm::pi<float>(), glm::vec3(0, 1, 0)),
			glm::scale(glm::mat4(1.0f), glm::vec3((float)obj["scale"], (float)obj["scale"], (float)obj["scale"])));

		envobj->model_ids.insert((int)obj["model_id"]);
		env_objs.push_back(envobj);
	}
	this->game = game;
	this->network = network;

	// Floor
	floor = new Model("../models/quad.obj", "../textures/floor.png", false);
	floor->localMtx = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 0.0f, 120.0f)) *
		glm::rotate(glm::mat4(1.0f), -90.0f / 180.0f * glm::pi<float>(), glm::vec3(1, 0, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(2));
}

void ClientScene::initialize_skills(ArcheType selected_type) {

	unordered_map<unsigned int, Skill>* skill_map = new unordered_map<unsigned int, Skill>();
	unordered_map<ArcheType, vector<unsigned int>> *archetype_skillset = new unordered_map<ArcheType, vector<unsigned int>>();

	Skill::load_archtype_data(skill_map, archetype_skillset);

	// get list of skills for archetype
	unordered_map<ArcheType, vector<unsigned int>>::iterator a_it = archetype_skillset->find(selected_type);
	vector<unsigned int> vec = a_it->second;

	// push each skill into personal skills
	for (auto skill_id : vec) {
		unordered_map<unsigned int, Skill>::iterator s_it = skill_map->find(skill_id);
		personal_skills.push_back(s_it->second);
	}

	skill_timers = vector<nanoseconds>(personal_skills.size(), nanoseconds::zero());
	animation_timer = nanoseconds::zero();
	player.modelType = selected_type;
}

void ClientScene::clean_up()
{
	delete(camera);
	delete(root);
	delete(staticShader);
	delete(animationShader);
}

void ClientScene::initialize_UI(GLFWwindow* window) {

	ctx = nk_glfw3_init(window, NK_GLFW3_DEFAULT);
	{const void *image; int w, h;
	struct nk_font_config cfg = nk_font_config(0);
	struct nk_font_atlas *atlas;
	nk_glfw3_font_stash_begin(&atlas);
	media.font_14 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/ProggyClean.ttf", 14.0f, &cfg);

	media.font_18 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/Roboto-Regular.ttf", 18.0f, &cfg);

	media.font_20 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/Roboto-Regular.ttf", 20.0f, &cfg);

	media.font_22 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/Roboto-Regular.ttf", 22.0f, &cfg);
	media.font_32 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/Roboto-Regular.ttf", 32.0f, &cfg);

	media.font_64 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/Roboto-Regular.ttf", 64.0f, &cfg);
	nk_glfw3_font_stash_end();
	}
	nk_style_set_font(ctx, &(media.font_22->handle));
	//create media
	media.mage = icon_load("../icon/mage_icon.png");
	media.assasin = icon_load("../icon/assasin_icon.png");
	media.king = icon_load("../icon/king_icon.png");
	media.warrior = icon_load("../icon/warrior_icon.png");

	media.mage_skills[2] = icon_load("../icon/skills/evade.png");
	media.mage_skills[3] = icon_load("../icon/skills/projectile.png");
	media.mage_skills[1] = icon_load("../icon/skills/mage-aoe.png");
	media.mage_skills[0] = icon_load("../icon/skills/mage-cone_aoe.png");
	media.warrior_skills[2] = icon_load("../icon/skills/evade.png");
	media.warrior_skills[3] = icon_load("../icon/skills/projectile.png");
	media.warrior_skills[1] = icon_load("../icon/skills/warrior-charge.png");
	media.warrior_skills[0] = icon_load("../icon/skills/warrior-aoe.png");
	media.assassin_skills[2] = icon_load("../icon/skills/evade.png");
	media.assassin_skills[3] = icon_load("../icon/skills/projectile.png");
	media.assassin_skills[1] = icon_load("../icon/skills/assassin-invisiblity.png");
	media.assassin_skills[0] = icon_load("../icon/skills/assassin-teleport.png");
	media.king_skills[2] = icon_load("../icon/skills/evade.png");
	media.king_skills[3] = icon_load("../icon/skills/projectile.png");
	media.king_skills[1] = icon_load("../icon/skills/king-aoe.png");
	media.king_skills[0] = icon_load("../icon/skills/king-silence.png");

}

void  ClientScene::text_input(GLFWwindow *win, unsigned int codepoint)
{
	(void)win;
	if (glfw.text_len < NK_GLFW_TEXT_MAX)
		glfw.text[glfw.text_len++] = codepoint;
}


GLFWwindow* ClientScene::create_window(int width, int height)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Create the GLFW window
	GLFWwindow* window = glfwCreateWindow(width, height, window_title, NULL, NULL);

	// Check if the window could not be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		fprintf(stderr, "Either GLFW is not installed or your graphics card does not support modern OpenGL.\n");
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window
	glfwMakeContextCurrent(window);

	// Set swap interval to 1
	glfwSwapInterval(1);

	// Get the width and height of the framebuffer to properly resize the window
	glfwGetFramebufferSize(window, &width, &height);
	// Call the resize callback to make sure things get drawn immediately
	//ClientScene::resize_callback(window, width, height);
	ClientScene::width = width;
	ClientScene::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);
	return window;
}

void ClientScene::updateTimers(nanoseconds timePassed) {
	// update individual skill timers
	for (int i = 0; i < skill_timers.size(); i++) {
		if (skill_timers[i] > nanoseconds::zero()) {
			skill_timers[i] -= timePassed;
			if (skill_timers[i] < nanoseconds::zero()) {
				skill_timers[i] = nanoseconds::zero();
				logger()->debug("skill {} is ready to fire!", i);
			}
		}
	}
	// update global animation timer
	if (animation_timer > nanoseconds::zero()) {
		animation_timer -= timePassed;
		if (animation_timer < nanoseconds::zero()) {
			animation_timer = nanoseconds::zero();
		}
	}

	// update skill duration
	if (skillDurationTimer > nanoseconds::zero()) {
		skillDurationTimer -= timePassed;
		if (skillDurationTimer <= nanoseconds::zero()) {
			// hardcode for assassin and king (only classes w duration skills)
			if (player.modelType == ASSASSIN) {
				ClientInputPacket endSkillPacket = game->createSkillPacket(NULL_POINT, 14);
				network->sendToServer(endSkillPacket);
			}
			if (player.modelType == KING) {
				logger()->debug("sent unsilence packet");
				ClientInputPacket unsilencePacket = game->createSkillPacket(NULL_POINT, 34);
				network->sendToServer(unsilencePacket);
			}
			skillDurationTimer = nanoseconds::zero();
		}
	}
}

void ClientScene::resize_callback(GLFWwindow* window, int width, int height)
{
	ClientScene::width = width;
	ClientScene::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		camera->SetAspect((float)width / (float)height);
	}
	if (width < 600) {
		nk_style_set_font(ctx, &(media.font_14->handle));
	} else if(width < 900) {
		nk_style_set_font(ctx, &(media.font_18->handle));
	}
	else if (width < 1200) {
		nk_style_set_font(ctx, &(media.font_22->handle));
	}
	else {
		nk_style_set_font(ctx, &(media.font_32->handle));
	}
}

void ClientScene::idle_callback()
{
	// Call the update function the cube
	//cube->update();
	time += 2.0 / 60;
	glm::mat4 playerNode = clientSceneGraphMap[player.root_id]->M;
	camera->cam_look_at = { playerNode[3][0],playerNode[3][1],playerNode[3][2] };
	camera->cam_pos = initCamPos + glm::vec3({ playerNode[3][0],playerNode[3][1],playerNode[3][2] });
	camera->Update();
	for (auto &model : models) {
		if(model.second.model->isAnimated)
			model.second.model->BoneTransform(2.0f / 60);
	}
}

void ClientScene::renderPreparePhase(GLFWwindow* window) {


	/* Input */
	glfwPollEvents();
	nk_glfw3_new_frame();
	prepare_layout(ctx, &media, ClientScene::width, ClientScene::height, &this->player);

	nk_glfw3_render(NK_ANTI_ALIASING_OFF, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

	// Swap buffers
	glfwSwapBuffers(window);
}

void ClientScene::renderLobbyPhase(GLFWwindow* window) {
	
	
	 /* Input */
	glfwPollEvents();
	nk_glfw3_new_frame();
	lobby_layout(ctx, &media, ClientScene::width, ClientScene::height, game);

	nk_glfw3_render(NK_ANTI_ALIASING_OFF, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

	// Swap buffers
	glfwSwapBuffers(window);
}

void ClientScene::renderKillPhase(GLFWwindow* window) {

	auto vpMatrix = camera->GetViewProjectMtx();

	// floor
	floor->draw(staticShader, glm::mat4(1.0f), vpMatrix);
	// players
	root->draw(models, glm::mat4(1.0f), vpMatrix, clientSceneGraphMap);
	// environment objects
	for (auto &env_obj : env_objs) {
		env_obj->draw(models, glm::mat4(1.0f), vpMatrix, clientSceneGraphMap);
	}
	 /* Input */
	glfwPollEvents();
	nk_glfw3_new_frame();

	/* GUI */

	kill_layout(ctx, &media, width, height, & this->player, skill_timers);
	/* ----------------------------------------- */


	nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);


	// Swap buffers
	glfwSwapBuffers(window);
}

void ClientScene::display_callback(GLFWwindow* window)
{
	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
	//glClear(GL_COLOR_BUFFER_BIT);
	//glClearColor(bg.r, bg.g, bg.b, bg.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (game->currPhase == ClientStatus::LOBBY) renderLobbyPhase(window);
	else if (game->currPhase == ClientStatus::KILL) renderKillPhase(window);
	else renderPreparePhase(window);
}

void ClientScene::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto log = logger();
	// are we even allowed to process input?
	if (animation_timer > nanoseconds::zero()) {
		return;
	}


	// Check for a key press
	if (action == GLFW_PRESS)
	{
		/*
			Q --> Directional Skill (with the exception of King)
			W --> Omni Directional Skill
			E --> Evade (omni)
			R --> Projectile (directional)
		    
			*******    In meta_data.json    ******
			Skills MUST be in the order of: evade (0), projectile (1), omni (2), directional (3)
		*/
		if (key == GLFW_KEY_ESCAPE)
		{
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		else if (key == GLFW_KEY_Q) // DIRECTIONAL SKILL		
		{

			// check cooldown
			if (skill_timers[DIR_SKILL_INDEX] > nanoseconds::zero()) {
				return;
			}

			// ONLY EXCEPTION: KING
			if (player.modelType == KING) {
				Skill subjugation = personal_skills[DIR_SKILL_INDEX];
				Skill adjustedSkill = Skill::calculateSkillBasedOnLevel(subjugation, subjugation.level);

				// set cooldown
				std::chrono::seconds sec((int)adjustedSkill.cooldown);
				skill_timers[DIR_SKILL_INDEX] = nanoseconds(sec);

				// hardcoded case for king (and assassin)
				if (player.modelType == KING) {
					// set duration for silence
					std::chrono::seconds sec((int)adjustedSkill.duration);
					skillDurationTimer = nanoseconds(sec);
				}
				ClientInputPacket subjugationPacket = game->createSkillPacket(NULL_POINT, adjustedSkill.skill_id);
				network->sendToServer(subjugationPacket);
				return;
			}

			// prep for left mouse click
			player.action_state = ACTION_DIRECTIONAL_SKILL;
			player.isPrepProjectile = false;
		}
		else if (key == GLFW_KEY_R) { // PROJECTILE SKILL
			// check cooldown
			if (skill_timers[PROJ_INDEX] > nanoseconds::zero()) {
				return;
			}
			// prep for left mouse click
			player.action_state = ACTION_DIRECTIONAL_SKILL;
			player.isPrepProjectile = true;
		}
		else if (key == GLFW_KEY_W) { // OMNIDIRECTIONAL SKILL
			// check cooldown
			if (skill_timers[OMNI_SKILL_INDEX] > nanoseconds::zero()) {
				return;
			}
			// cancel prep mode if necessary
			player.action_state = ACTION_MOVEMENT;

			// get skill and adjust
			Skill omniSkill = personal_skills[OMNI_SKILL_INDEX];
			Skill adjustedSkill = Skill::calculateSkillBasedOnLevel(omniSkill, omniSkill.level);
			
			// set cooldown
			std::chrono::seconds sec((int)adjustedSkill.cooldown);
			skill_timers[OMNI_SKILL_INDEX] = nanoseconds(sec);

			// hardcoded case for assassin (and king)
			if (player.modelType == ASSASSIN) {
				// set duration for invisibility / minimap skill
				std::chrono::seconds sec((int)adjustedSkill.duration);
				skillDurationTimer = nanoseconds(sec);
			}
			
			// send server skill packet
			ClientInputPacket omniSkillPacket = game->createSkillPacket(NULL_POINT, adjustedSkill.skill_id);
			network->sendToServer(omniSkillPacket);
		}
		else if (key == GLFW_KEY_E) { // EVADE SKILL
			if (skill_timers[EVADE_INDEX] > nanoseconds::zero()) {
				return;
			}
			// cancel prep mode if necessary
			player.action_state = ACTION_MOVEMENT;

			// get skill and adjust
			Skill evadeSkill = personal_skills[EVADE_INDEX];
			Skill adjustedSkill = Skill::calculateSkillBasedOnLevel(evadeSkill, evadeSkill.level);

			// set cooldown
			std::chrono::seconds sec((int)adjustedSkill.cooldown);
			skill_timers[EVADE_INDEX] = nanoseconds(sec);

			// send server skill packet
			ClientInputPacket evadeSkillPacket = game->createSkillPacket(NULL_POINT, adjustedSkill.skill_id);
			network->sendToServer(evadeSkillPacket);
		}
	}
}

void ClientScene::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	glm::vec3 z_dir = camera->cam_look_at - camera->cam_pos;
	if (!((camera->cam_pos.y < min_scroll && yoffset > 0) || (camera->cam_pos.y > max_scroll && yoffset < 0)))
		initCamPos -= ((float)-yoffset * glm::normalize(z_dir));
}

void ClientScene::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{

		// player moving
			double xpos, ypos;
			//getting cursor position
			glfwGetCursorPos(window, &xpos, &ypos);
			//printf("Cursor Position at %f: %f \n", xpos, ypos);
			glm::vec3 new_dest = viewToWorldCoordTransform(xpos, ypos);
			ClientInputPacket movementPacket = game->createMovementPacket(new_dest);
			network->sendToServer(movementPacket);
		// player shooting projectile

	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (player.action_state == ACTION_DIRECTIONAL_SKILL) {
			// get the right skill based on what skill was prepped (projectile vs directional skill)
			Skill skill = player.isPrepProjectile ? personal_skills[PROJ_INDEX] : personal_skills[DIR_SKILL_INDEX];
			Skill adjustedSkill = Skill::calculateSkillBasedOnLevel(skill, skill.level);
			
			// set cooldown
			std::chrono::seconds sec((int)adjustedSkill.cooldown);
			if (player.isPrepProjectile) {
				skill_timers[PROJ_INDEX] = nanoseconds(sec);
				// hardcode assassin: on firing projectile, you instantly cancel invisibility if active
				if (player.modelType == ASSASSIN && skillDurationTimer > nanoseconds::zero()) {
					ClientInputPacket cancelInvisibilityPacket = game->createSkillPacket(NULL_POINT, personal_skills[OMNI_SKILL_INDEX].skill_id);
					network->sendToServer(cancelInvisibilityPacket);
					skillDurationTimer = nanoseconds::zero();
				}
			}
			else {
				skill_timers[ACTION_DIRECTIONAL_SKILL] = nanoseconds(sec);
			}

			// get cursor position and translate it to world point
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			glm::vec3 new_dest = viewToWorldCoordTransform(xpos, ypos);

			// create skill packet and send to server
			ClientInputPacket skillPacket = game->createSkillPacket(new_dest, adjustedSkill.skill_id);
			logger()->debug("sending server skill packet w id of {}", adjustedSkill.skill_id);
			network->sendToServer(skillPacket);
			player.action_state = ACTION_MOVEMENT;
		}
	}
}

// SCREEN SPACE: mouse_x and mouse_y are screen space
glm::vec3 ClientScene::viewToWorldCoordTransform(int mouse_x, int mouse_y) {
	// NORMALISED DEVICE SPACE
	double x = 2.0 * mouse_x / width - 1;
	double y = 2.0 * mouse_y / height - 1;

	// HOMOGENEOUS SPACE
	double depth = camera->GetDepth();
	glm::vec4 screenPos = glm::vec4(x, -y, depth, 1.0f);

	// Projection/Eye Space
	glm::mat4 ProjectView = camera->GetViewProjectMtx();
	glm::mat4 viewProjectionInverse = inverse(ProjectView);

	glm::vec4 worldPos = viewProjectionInverse * screenPos;
	//printf("world pos map to: %f %f %f\n", worldPos.x, worldPos.y, worldPos.z);
	glm::vec3 realPos = glm::vec3(worldPos.x / worldPos.w, worldPos.y / worldPos.w, worldPos.z / worldPos.w);
	

	glm::vec3 cam_pos = camera->cam_pos;
	glm::vec3 dir = glm::normalize(realPos - cam_pos);
	float n = -cam_pos.y / dir.y;
	realPos.x = cam_pos.x + n * dir.x;
	realPos.y = 0;
	realPos.z = cam_pos.z + n * dir.z;

	//printf("world pos remap to: %f %f %f\n", realPos.x, realPos.y, realPos.z);

	return realPos;
}


/*
	Deserialize init scene packet from server, initialize player_id, root_id, root. 
*/
void ClientScene::handleInitScenePacket(char * data) {
	memcpy(&player.player_id, data, sizeof(unsigned int));
	data += sizeof(unsigned int);
	memcpy(&player.root_id, data, sizeof(unsigned int));
	data += sizeof(unsigned int);
	root = Serialization::deserializeSceneGraph(data, clientSceneGraphMap);
}

/*
	Deserialize updated scene graph from server.
*/
void ClientScene::handleServerTickPacket(char * data) {
	root = Serialization::deserializeSceneGraph(data, clientSceneGraphMap);
	// nullify invisibility state if you're assassin (duh)
	if (player.modelType == ASSASSIN) {
		clientSceneGraphMap[player.root_id]->enabled = true;
	}
}

void ClientScene::setRoot(Transform * newRoot) {
	root = newRoot;
}

