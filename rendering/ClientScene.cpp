#include "ClientScene.h"
#include "nlohmann\json.hpp"
#include "GUILayout.h"
#include <fstream>

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#define VULNERABLE -2
#define UNEVADE -1
#define EVADE_INDEX 0
#define PROJ_INDEX 1
#define OMNI_SKILL_INDEX 2
#define DIR_SKILL_INDEX 3
#define UNSILENCE 34
#define END_SILENCE_HEMISPHERE 35
#define VISIBILITY 14
#define UNSPRINT 15
#define RESPAWN_TIME 3
#define TEXT_SHOW_TIME 3
#define INVINCIBILITY_TIME 1

using json = nlohmann::json;
struct nk_context * ctx;
struct nk_colorf bg = { 0.1f,0.1f,0.1f,1.0f };
ClientScene * Window_static::scene = new ClientScene();
struct media media;
struct guiStatus guiStatuses;
GLuint loadTexture(const char * imagepath) {
	int width, height, n;
	// Actual RGB data
	unsigned char * data;

	data = stbi_load(imagepath, &width, &height, &n, STBI_rgb_alpha);
	if (!data) {
		if (!data) printf("[Particle]: failed to load image: %s", imagepath);
	}
	GLuint textureID;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	return textureID;
}

void ClientScene::resetGUIStatus() {
	guiStatuses.betAmount = 0;
	guiStatuses.currPrepareLayout = 0;
	guiStatuses.shopCategory = 0;
	guiStatuses.killUpdates.clear();
	guiStatuses.killStreakUpdates.clear();
}

void ClientScene::initialize_objects(ClientGame * game, ClientNetwork * network, LeaderBoard* leaderBoard, 
	list<int>* killstreak_data)
{
	camera = new Camera();
	camera->SetAspect(width / height);
	camera->Reset();

	animationShader = new Shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
	staticShader = new Shader(TOON_VERTEX_SHADER_PATH, TOON_FRAGMENT_SHADER_PATH);
	particleShader = new Shader(PARTICLE_VERTEX_SHADER_PATH, PARTICLE_FRAGMENT_SHADER_PATH);
	circleShader = new Shader(CIRCLE_VERTEX_SHADER_PATH, CIRCLE_FRAGMENT_SHADER_PATH);
	particleTexture = loadTexture("../textures/flame.png");
	ifstream json_model_paths("../model_paths.json");
	json pathObjs = json::parse(json_model_paths);
	for (auto & obj : pathObjs["data"]) {
		if (obj["animated"]) {
			models[(unsigned int)obj["model_id"]] = ModelData{ 
				new Model(obj["path"], obj["texture_path"], true), 
				glm::vec4((float)(obj["color_rgb"][0]), (float)(obj["color_rgb"][1]), (float)(obj["color_rgb"][2]), 1.0f), 
				animationShader, 
				TEXTURE, 
				0 
			};
			for (unsigned int i = 0; i < 8; i++) {
				models[(unsigned int)obj["model_id"]].model->animation_frames.push_back(vector<float>{ ((unsigned int)obj["animations"][i][0]) / 30.0f * (unsigned int)obj["ticks_per_second"], ((unsigned int)obj["animations"][i][1]) / 30.0f * (unsigned int)obj["ticks_per_second"]});
			}
		}
		else if ((unsigned int)obj["model_id"] == 301) {
			models[(unsigned int)obj["model_id"]] = ModelData{ new Model(obj["path"], obj["texture_path"], false), glm::vec4((float)(obj["color_rgb"][0]), (float)(obj["color_rgb"][1]), (float)(obj["color_rgb"][2]), 0.2f), staticShader, COLOR, 0 };
		}
		// the sphere for king's silence
		else {
			models[(unsigned int)obj["model_id"]] = ModelData{ new Model(obj["path"], obj["texture_path"], false), glm::vec4((float)(obj["color_rgb"][0]), (float)(obj["color_rgb"][1]), (float)(obj["color_rgb"][2]), 1.0f), staticShader, TEXTURE, 0 };
			//sphere = models[(unsigned int)obj["model_id"]].model;
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
	this->leaderBoard = leaderBoard;
	this->killstreak_data = killstreak_data;

	// Floor
	floor = new Model("../models/quad.obj", "../textures/floor.png", false);
	floor->localMtx = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 0.0f, 120.0f)) *
		glm::rotate(glm::mat4(1.0f), -90.0f / 180.0f * glm::pi<float>(), glm::vec3(1, 0, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(200));

	// Circle and arrow of directional skill rendering
	range = new Circle();
	range->localMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, -0.8f));
	arrow = new Model("../models/quad.obj", "../textures/arrow.png", false);
	arrow->localMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 10.0f)) *
		glm::rotate(glm::mat4(1.0f), -90.0f / 180.0f * glm::pi<float>(), glm::vec3(1, 0, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(5));
	cross = new Model("../models/quad.obj", "../textures/cross.png", false);
	cross->localMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.1f, 2.0f, 4.3f)) *
		glm::rotate(glm::mat4(1.0f), -90.0f / 180.0f * glm::pi<float>(), glm::vec3(1, 0, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(3));
}


/*
	Reset scene elements before kill phase.
*/
void ClientScene::resetPreKillPhase()
{
	// reset directional skill input
	player.action_state = ACTION_MOVEMENT; 
	player.isPrepProjectile = false;

	// reset all timers
	for (auto & timer : skill_timers) {
		timer = nanoseconds::zero();
	}
	respawn_timer = nanoseconds::zero();
	animation_timer = nanoseconds::zero();
	skillDurationTimer = nanoseconds::zero();
	evadeDurationTimer = nanoseconds::zero();
	sprintDurationTimer = nanoseconds::zero();
	invincibilityTimer = nanoseconds::zero();
	kingSilenceHemisphereTimer = nanoseconds::zero();
	isCharging = false;

	// interrupt death animation if necessary
	for (auto & model : models) {
		if (model.second.model->isAnimated) {
			auto & modelPtr = model.second.model;
			modelPtr->isPlayingActiveAnimation = false;
			modelPtr->curr_mode = idle;
			modelPtr->movementMode = idle;
			modelPtr->animationMode = -1;
		}
	}
	// reset evade
	clientSceneGraphMap[player.root_id]->isEvading = false;

	// reset invisibility
	clientSceneGraphMap[player.root_id]->isInvisible = false;

	// reset invincibility
	player.isInvincible = false;
	clientSceneGraphMap[player.root_id]->isInvincible = false;
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
		player.availableSkills.push_back(s_it->second);
	}

	skill_timers = vector<nanoseconds>(player.availableSkills.size(), nanoseconds::zero());
	animation_timer = nanoseconds::zero();
	respawn_timer = nanoseconds::zero();
	skillDurationTimer = nanoseconds::zero();
	evadeDurationTimer = nanoseconds::zero();
	sprintDurationTimer = nanoseconds::zero();
	invincibilityTimer = nanoseconds::zero();
	kingSilenceHemisphereTimer = nanoseconds::zero();
	player.modelType = selected_type;
}

void ClientScene::clean_up()
{
	delete(camera);
	delete(root);
	delete(floor);
	delete(range);
	delete(arrow);
	delete(cross);
	delete(staticShader);
	delete(animationShader);
	delete(particleShader);
	delete(circleShader);
}

void ClientScene::initialize_UI(GLFWwindow* window) {

	ctx = nk_glfw3_init(window, NK_GLFW3_DEFAULT);
	{const void *image; int w, h;
	struct nk_font_config cfg = nk_font_config(0);
	struct nk_font_atlas *atlas;
	nk_glfw3_font_stash_begin(&atlas);
	media.font_14 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/monogram_extended.ttf", 14.0f, &cfg);

	media.font_18 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/monogram_extended.ttf", 18.0f, &cfg);

	media.font_20 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/monogram_extended.ttf", 20.0f, &cfg);
	media.font_22 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/monogram_extended.ttf", 22.0f, &cfg);
	media.font_32 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/monogram_extended.ttf", 32.0f, &cfg);
	media.font_48 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/monogram_extended.ttf", 48.0f, &cfg);

	media.font_64 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/monogram_extended.ttf", 64.0f, &cfg);

	media.font_128 = nk_font_atlas_add_from_file(atlas, "../nuklear-master/extra_font/monogram_extended.ttf", 128.0f, &cfg);
	nk_glfw3_font_stash_end();
	}
	glfw.atlas.default_font = media.font_32;
	nk_style_set_font(ctx, &(media.font_32->handle));
	//create media
	media.mage = icon_load("../icon/mage_icon.png");
	media.assasin = icon_load("../icon/assassin_icon.png");
	media.king = icon_load("../icon/king_icon.png");
	media.warrior = icon_load("../icon/warrior_icon.png");

	media.points = icon_load("../icon/trophy.png");
	media.gold = icon_load("../icon/gold.png");
	media.lobby_background = icon_load("../icon/lobbybg.jpg");
	media.prepare_background = icon_load("../icon/preparebg.jpeg");
	media.mage_skills[2] = icon_load("../icon/skills/evade.png");
	media.mage_skills[3] = icon_load("../icon/skills/projectile.png");
	media.mage_skills[1] = icon_load("../icon/skills/mage-aoe.png");
	media.mage_skills[0] = icon_load("../icon/skills/mage-cone_aoe.png");
	media.mage_silenced[1] = icon_load("../icon/skills/mage-aoe-silenced.png");
	media.mage_silenced[0] = icon_load("../icon/skills/mage-cone_aoe-silenced.png");
	media.warrior_skills[2] = icon_load("../icon/skills/evade.png");
	media.warrior_skills[3] = icon_load("../icon/skills/projectile.png");
	media.warrior_skills[1] = icon_load("../icon/skills/warrior-charge.png");
	media.warrior_skills[0] = icon_load("../icon/skills/warrior-aoe.png");
	media.warrior_silenced[1] = icon_load("../icon/skills/warrior-charge-silenced.png");
	media.warrior_silenced[0] = icon_load("../icon/skills/warrior-aoe-silenced.png");
	media.assassin_skills[2] = icon_load("../icon/skills/evade.png");
	media.assassin_skills[3] = icon_load("../icon/skills/projectile.png");
	media.assassin_skills[1] = icon_load("../icon/skills/assassin-invisiblity.png");
	media.assassin_skills[0] = icon_load("../icon/skills/assassin-teleport.png");
	media.assassin_silenced[1] = icon_load("../icon/skills/assassin-invisiblity-silenced.png");
	media.assassin_silenced[0] = icon_load("../icon/skills/assassin-teleport-silenced.png");
	media.king_skills[2] = icon_load("../icon/skills/evade.png");
	media.king_skills[3] = icon_load("../icon/skills/projectile.png");
	media.king_skills[1] = icon_load("../icon/skills/king-aoe.png");
	media.king_skills[0] = icon_load("../icon/skills/king-silence.png");
	media.king_silenced[1] = icon_load("../icon/skills/king-aoe.png");
	media.king_silenced[0] = icon_load("../icon/skills/king-silence.png");

	guiStatuses.betAmount = 0;
	guiStatuses.currPrepareLayout = 0;
	guiStatuses.shopCategory = 0;
}

void  ClientScene::text_input(GLFWwindow *win, unsigned int codepoint)
{
	(void)win;
	if (glfw.text_len < NK_GLFW_TEXT_MAX)
		glfw.text[glfw.text_len++] = codepoint;
}


GLFWwindow* ClientScene::create_window()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);
	auto monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	//glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	//glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	//glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	//glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	int width = mode->width;
	int height = mode->height;
	GLFWwindow* window = glfwCreateWindow(width, height, window_title, NULL, NULL);


	// Create the GLFW window
	//GLFWwindow* window = glfwCreateWindow(width, height, window_title, glfwGetPrimaryMonitor(), NULL);

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
	//update kill streak timers
	auto streakIter = guiStatuses.killStreakUpdates.begin();
	for (; streakIter != guiStatuses.killStreakUpdates.end();) {
		auto & ns = (*streakIter).second;
		ns -= timePassed;
		if (ns < nanoseconds::zero()) {
			streakIter = guiStatuses.killStreakUpdates.erase(streakIter);
		}
		else {
			++streakIter;
		}
	}

	// update individual skill timers
	for (int i = 0; i < skill_timers.size(); i++) {
		if (skill_timers[i] > nanoseconds::zero()) {
			skill_timers[i] -= timePassed;
			if (skill_timers[i] < nanoseconds::zero()) {
				skill_timers[i] = nanoseconds::zero();
				audio.play(glm::vec3(0), COOLDOWN_RESET_AUDIO);
				//logger()->debug("skill {} is ready to fire!", i);
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

	// update respawn timer
	if (respawn_timer > nanoseconds::zero())
	{
		respawn_timer -= timePassed;
		if (respawn_timer <= nanoseconds::zero()) {		
			// send respawn packet to server
			ClientInputPacket respawnPacket = game->createRespawnPacket();
			network->sendToServer(respawnPacket);
			respawn_timer = nanoseconds::zero();
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

	// update evade duration
	if (evadeDurationTimer > nanoseconds::zero()) {
		evadeDurationTimer -= timePassed;
		if (evadeDurationTimer <= nanoseconds::zero()) {
			ClientInputPacket unevadePacket = game->createSkillPacket(NULL_POINT, UNEVADE);
			network->sendToServer(unevadePacket);
			evadeDurationTimer = nanoseconds::zero();
		}
	}

	// update sprint duration
	if (sprintDurationTimer > nanoseconds::zero()) {
		sprintDurationTimer -= timePassed;
		if (sprintDurationTimer <= nanoseconds::zero()) {
			ClientInputPacket unsprintPacket = game->createSkillPacket(NULL_POINT, UNSPRINT);
			network->sendToServer(unsprintPacket);
			sprintDurationTimer = nanoseconds::zero();
		}
	}

	// update invincibility duration
	if (invincibilityTimer > nanoseconds::zero()) {
		invincibilityTimer -= timePassed;
		if (invincibilityTimer <= nanoseconds::zero()) {
			ClientInputPacket vulnerablePacket = game->createSkillPacket(NULL_POINT, VULNERABLE);
			network->sendToServer(vulnerablePacket);
			invincibilityTimer = nanoseconds::zero();
		}
	}

	// update king silence hemisphere duration
	if (kingSilenceHemisphereTimer > nanoseconds::zero()) {
		kingSilenceHemisphereTimer -= timePassed;
		if (kingSilenceHemisphereTimer <= nanoseconds::zero()) {
			ClientInputPacket disappearHemispherePacket = game->createSkillPacket(NULL_POINT, END_SILENCE_HEMISPHERE);
			network->sendToServer(disappearHemispherePacket);
			kingSilenceHemisphereTimer = nanoseconds::zero();
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
		glfw.atlas.default_font = media.font_14;
		nk_style_set_font(ctx, &(media.font_14->handle));
	} else if(width < 900) {
		glfw.atlas.default_font = media.font_18;
		nk_style_set_font(ctx, &(media.font_18->handle));
	}
	else if (width < 1200) {
		glfw.atlas.default_font = media.font_22;
		nk_style_set_font(ctx, &(media.font_22->handle));
	}
	else {
		glfw.atlas.default_font = media.font_32;
		nk_style_set_font(ctx, &(media.font_32->handle));

	}
}

void ClientScene::idle_callback()
{
	// Call the update function the cube
	//cube->update();
	
	camera->Update();
	for (auto &model : models) {
		if (model.second.model->isAnimated) {
			float timeToIncrement = model.first == 4 ? 2.0f / 60.0f * 33.0f : 2.0f / 60.0f;
			model.second.model->BoneTransform(timeToIncrement);
		}
	}

	for (auto &node : clientSceneGraphMap) {
		node.second->clientUpdate();
	}
}

void ClientScene::renderPreparePhase(GLFWwindow* window) {


	/* Input */
	glfwPollEvents();
	nk_glfw3_new_frame();
	prepare_layout(ctx, &media, ClientScene::width, ClientScene::height, &this->player, leaderBoard,usernames, archetypes,game, guiStatuses);


	nk_glfw3_render(NK_ANTI_ALIASING_OFF, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

	// Swap buffers
	glfwSwapBuffers(window);
}

void ClientScene::renderFinalPhase(GLFWwindow* window) {
	/* Input */
	glfwPollEvents();
	nk_glfw3_new_frame();
	winner_layout(ctx, &media, ClientScene::width, ClientScene::height, leaderBoard, usernames, archetypes, game);


	nk_glfw3_render(NK_ANTI_ALIASING_OFF, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

	// Swap buffers
	glfwSwapBuffers(window);
}
 
 void ClientScene::renderSummaryPhase(GLFWwindow* window) {
	 /* Input */
	glfwPollEvents();
	nk_glfw3_new_frame();
	summary_layout(ctx, &media, ClientScene::width, ClientScene::height, leaderBoard, usernames, archetypes, game);


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

	// environment objects
	for (auto &env_obj : env_objs) {
		env_obj->draw(models, glm::mat4(1.0f), vpMatrix, clientSceneGraphMap);
	}

	// floor
	floor->draw(staticShader, glm::mat4(1.0f), vpMatrix);

	// players
	root->draw(models, glm::mat4(1.0f), vpMatrix, clientSceneGraphMap);

	// directional skill
	if (player.action_state == ACTION_DIRECTIONAL_SKILL) {
		auto personal_skills = getPlayerSkills();
		Skill & skill = player.isPrepProjectile ? personal_skills[PROJ_INDEX] : personal_skills[DIR_SKILL_INDEX];
		Skill & adjustedSkill = Skill::calculateSkillBasedOnLevel(skill, skill.level);
		float radius = adjustedSkill.range;
		range->createCircle(radius);

		glm::vec3 playerPos = glm::vec3(clientSceneGraphMap[player.root_id]->M[3][0], clientSceneGraphMap[player.root_id]->M[3][1], clientSceneGraphMap[player.root_id]->M[3][2]);
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		glm::vec3 convertedPos = viewToWorldCoordTransform(xpos, ypos);
		if (player.modelType == MAGE && !player.isPrepProjectile) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			range->draw(circleShader, glm::translate(glm::mat4(1.0f), playerPos), vpMatrix);
			cross->draw(staticShader, glm::translate(glm::mat4(1.0f), playerPos) * glm::translate(glm::mat4(1.0f), convertedPos - playerPos), vpMatrix);
			glDisable(GL_BLEND);
		}
		else {
			glm::vec3 direction = glm::normalize(convertedPos - playerPos);
			float angle = glm::acos(glm::dot(direction, glm::vec3(0, 0, 1)));
			glm::vec3 axis = glm::cross(direction, glm::vec3(0, 0, -1));
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			range->draw(circleShader, glm::translate(glm::mat4(1.0f), playerPos), vpMatrix);
			arrow->draw(staticShader, glm::translate(glm::mat4(1.0f), playerPos) * glm::rotate(glm::mat4(1.0f), angle, axis), vpMatrix);
			glDisable(GL_BLEND);
		}
	}

	/*staticShader->use();
	staticShader->setInt("UseTex", 0);
	staticShader->setVec4("color", glm::vec4(0.621, 0.527, 0.6836, 0.5));
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	sphere->draw(staticShader, glm::translate(glm::mat4(1.0f), glm::vec3(0, 5, 0)), vpMatrix);
	glDisable(GL_BLEND);*/

	 /* Input */
	glfwPollEvents();
	nk_glfw3_new_frame();

	/* GUI */


	kill_layout(ctx, &media, width, height, & this->player, skill_timers, leaderBoard, usernames, archetypes, killTextDeterminant, game, guiStatuses);

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
	else if (game->currPhase == ClientStatus::PREPARE) renderPreparePhase(window);
	else if (game->currPhase == ClientStatus::FINAL) renderFinalPhase(window);
	else renderSummaryPhase(window);
}

void ClientScene::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto log = logger();
	// are we even allowed to process input?
	if (animation_timer > nanoseconds::zero() || respawn_timer > nanoseconds::zero()) {
		return;
	}

	if (checkInAnimation()) return;

	if (isCharging && player.modelType == WARRIOR) return;

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
			// glfwSetWindowShouldClose(window, GL_TRUE);
			player.action_state = ACTION_MOVEMENT;
			player.isPrepProjectile = false;
		}
		else if (key == GLFW_KEY_Q) // DIRECTIONAL SKILL		
		{

			// check cooldown
			if (skill_timers[DIR_SKILL_INDEX] > nanoseconds::zero()) {
				return;
			}

			// EXCEPTIONS: KING AND ASSASSIN
			if (player.modelType == KING || player.modelType == ASSASSIN) {
				Skill skill = player.availableSkills[DIR_SKILL_INDEX];
				Skill adjustedSkill = Skill::
					calculateSkillBasedOnLevel(skill, skill.level);

				// set cooldown
				if (!player.isSilenced) {
					logger()->debug("q key cooldown set");
					std::chrono::milliseconds ms(adjustedSkill.cooldown);
					skill_timers[DIR_SKILL_INDEX] = nanoseconds(ms);
				}

				std::chrono::milliseconds ms(adjustedSkill.duration);
				// set duration for silence / sprint
				if (player.modelType == KING) {
					skillDurationTimer = nanoseconds(ms);
					kingSilenceHemisphereTimer = nanoseconds(std::chrono::milliseconds(1500));
				}
				else {
					sprintDurationTimer = nanoseconds(ms);
				}

				ClientInputPacket skillPacket = game->createSkillPacket(NULL_POINT, adjustedSkill.skill_id);
				network->sendToServer(skillPacket);
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
			Skill omniSkill = player.availableSkills[OMNI_SKILL_INDEX];
			Skill adjustedSkill = Skill::calculateSkillBasedOnLevel(omniSkill, omniSkill.level);
			
			// set cooldown
			if (!player.isSilenced) {
				logger()->debug("w key cooldown set");
				std::chrono::milliseconds ms(adjustedSkill.cooldown);
				skill_timers[OMNI_SKILL_INDEX] = nanoseconds(ms);
			}

			// hardcoded case for assassin
			if (player.modelType == ASSASSIN && !player.isSilenced) {
				// play localized sound
				audio.play(glm::vec3(0), ASSASSIN_STEALTH_AUDIO);

				// set duration for invisibility
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
			Skill evadeSkill = player.availableSkills[EVADE_INDEX];
			Skill adjustedSkill = Skill::calculateSkillBasedOnLevel(evadeSkill, evadeSkill.level);

			// set cooldown
			std::chrono::milliseconds ms(adjustedSkill.cooldown);
			skill_timers[EVADE_INDEX] = nanoseconds(ms);

			// set duration timer
			evadeDurationTimer = nanoseconds(std::chrono::milliseconds(adjustedSkill.duration));

			// play sound
			audio.play(glm::vec3(0), SKELETON_EVADE_2_AUDIO);

			// send server skill packet
			ClientInputPacket evadeSkillPacket = game->createSkillPacket(NULL_POINT, adjustedSkill.skill_id);
			network->sendToServer(evadeSkillPacket);
		}
	}
}

void ClientScene::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	glm::vec3 z_dir = camera->cam_look_at - camera->cam_pos;
	if (!((camera->cam_pos.y < min_scroll && yoffset > 0) || (camera->cam_pos.y > max_scroll && yoffset < 0)))
		camera->cam_look_at -= ((float)-yoffset * glm::normalize(z_dir));
}

void ClientScene::playPreparePhaseBGM() {
	audio.play(glm::vec3(0), PREPARE_PHASE_MUSIC);
}

void ClientScene::playKillPhaseBGM() {
	audio.play(glm::vec3(0), KILL_PHASE_MUSIC);
}

void ClientScene::playFinalRoundBGM() {
	audio.play(glm::vec3(0), FINAL_ROUND_MUSIC);
}

void ClientScene::playCountdown() {
	audio.play(glm::vec3(0), TIMER_AUDIO);
}

void ClientScene::playButtonPress() {
	audio.play(glm::vec3(0), BUTTON_PRESS_AUDIO);
}

void ClientScene::playInvalidButtonPress() {
	audio.play(glm::vec3(0), CANNOT_BUY_ITEM_AUDIO);
}

void ClientScene::playChaching() {
	audio.play(glm::vec3(0), BUY_ITEM_1_AUDIO);
}

void ClientScene::playInvest() {
	audio.play(glm::vec3(0), BUY_ITEM_2_AUDIO);
}

void ClientScene::playRoundOver() {
	audio.play(glm::vec3(0), GAME_OVER_AUDIO);
}

void ClientScene::playKillStreak() {
	audio.play(glm::vec3(0), KILLSTREAK_AUDIO);
}

void ClientScene::playShutdown() {
	audio.play(glm::vec3(0), SHUTDOWN_AUDIO);
}

void ClientScene::playVictory() {
	audio.play(glm::vec3(0), VICTORY_AUDIO);
}


void ClientScene::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (respawn_timer > nanoseconds::zero()) return;		// player dead disable input
	if (game->currPhase == KILL && checkInAnimation()) return;

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{

		// player moving
			double xpos, ypos;
			//getting cursor position
			glfwGetCursorPos(window, &xpos, &ypos);
			//printf("Cursor Position at %f: %f \n", xpos, ypos);
			glm::vec3 new_dest = viewToWorldCoordTransform(xpos, ypos);
			//printf("Player's next Pos will be: %f, %f, %f \n", new_dest.x, new_dest.y, new_dest.z);
			ClientInputPacket movementPacket = game->createMovementPacket(new_dest);
			network->sendToServer(movementPacket);
		// player shooting projectile

	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (player.action_state == ACTION_DIRECTIONAL_SKILL) {
			// get the right skill based on what skill was prepped (projectile vs directional skill)
			Skill skill = player.isPrepProjectile ? player.availableSkills[PROJ_INDEX] : player.availableSkills[DIR_SKILL_INDEX];
			Skill adjustedSkill = Skill::calculateSkillBasedOnLevel(skill, skill.level);
			
			// set cooldown
			std::chrono::milliseconds ms(adjustedSkill.cooldown);
			if (player.isPrepProjectile) {
				if (!player.isSilenced) {
					logger()->debug("left key cooldown set");
					skill_timers[PROJ_INDEX] = nanoseconds(ms);
				}
				// hardcode assassin: on firing projectile, you instantly cancel invisibility if active
				if (player.modelType == ASSASSIN && skillDurationTimer > nanoseconds::zero()) {
					ClientInputPacket cancelInvisibilityPacket = game->createSkillPacket(NULL_POINT, VISIBILITY);
					network->sendToServer(cancelInvisibilityPacket);
					skillDurationTimer = nanoseconds::zero();
				}
			}
			else {
				if (!player.isSilenced) {
					logger()->debug("left key cooldown set");
					skill_timers[DIR_SKILL_INDEX] = nanoseconds(ms);
				}
			}

			// get cursor position and translate it to world point
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			glm::vec3 new_dest = viewToWorldCoordTransform(xpos, ypos);

			if (player.modelType == WARRIOR) isCharging = true;
			// create skill packet and send to server
			ClientInputPacket skillPacket = game->createSkillPacket(new_dest, adjustedSkill.skill_id);
			//logger()->debug("sending server skill packet w id of {}", adjustedSkill.skill_id);
			network->sendToServer(skillPacket);
			player.action_state = ACTION_MOVEMENT;
			player.isPrepProjectile = false;
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
int ClientScene::handleInitScenePacket(char * data) {

	// deserialize client id
	int client_id = -1;
	memcpy(&client_id, data, sizeof(unsigned int));
	data += sizeof(unsigned int);

	// deserialize usernames
	for (int i = 0; i < GAME_SIZE; i++) 
	{
		char username[16] = { 0 };
		memcpy(&username, data, 16);

		string username_str = (string) username;
		usernames.push_back(username_str);
		data += 16;
	}

	// deserialize archetypes
	for (int i = 0; i < GAME_SIZE; i++)
	{
		int d_type = -1;
		memcpy(&d_type, data, sizeof(int));

		ArcheType cur_type = static_cast<ArcheType>(d_type);
		archetypes.push_back(cur_type);
		data += sizeof(int);
	}

	memcpy(&player.player_id, data, sizeof(unsigned int));
	data += sizeof(unsigned int);
	memcpy(&player.root_id, data, sizeof(unsigned int));
	data += sizeof(unsigned int);
	root = Serialization::deserializeSceneGraph(data, clientSceneGraphMap, particleTexture, particleShader);

	audio.initListener(glm::vec3(0));
	playKillPhaseBGM();

	return client_id;
}

/*
	Deserialize updated scene graph & leaderboard from server.
*/
void ClientScene::handleServerTickPacket(char * data) {
	unsigned int sz = 0;

	// deserialize if they client is alive/dead
	bool server_alive = false;
	memcpy(&server_alive, data, sizeof(server_alive));
	sz += sizeof(server_alive);
	data += sizeof(server_alive);

	// Server tells client they died --> set respawn time
	if ( !server_alive && player.isAlive )		
	{
		player.isAlive = false;
		std::chrono::seconds sec((int)RESPAWN_TIME);

		respawn_timer = nanoseconds(sec);
		killTextDeterminant = rand() % KILLED_TEXT_NUM;

		// reset cooldowns
		for (int i = 0; i < skill_timers.size(); i++) skill_timers[i] = nanoseconds::zero();
	}

	// server respawning player (they're alive); client still thinks they're dead
	else if ( server_alive && !player.isAlive) {
		player.isAlive = true;
		// start the invincibility timer
		std::chrono::seconds sec((int)INVINCIBILITY_TIME);
		invincibilityTimer = nanoseconds(sec);

	}

	// deserialize client gold
	memcpy(&player.gold, data, sizeof(int));
	sz += sizeof(int);
	data += sizeof(int);

	//deserialize silence
	memcpy(&player.isSilenced, data, sizeof(bool));
	sz += sizeof(bool);
	data += sizeof(bool);

	// deserialize audio
	int numSounds = 0;
	memcpy(&numSounds, data, sizeof(int));
	sz += sizeof(int);
	data += sizeof(int);

	for (int i = 0; i < numSounds; i++) {
		int soundToPlay = 0;
		memcpy(&soundToPlay, data, sizeof(int));
		sz += sizeof(int);
		data += sizeof(int);

		// play the sound
		audio.play(glm::vec3(0), soundToPlay);
	}

	/*int currKill = INT_MAX;
	if (isCharging) currKill = leaderBoard->currentKills[player.player_id];*/

	// deserialize charge
	memcpy(&isCharging, data, sizeof(bool));
	sz += sizeof(bool);
	data += sizeof(bool);
	clientSceneGraphMap[player.root_id]->isCharging = isCharging;

	//deserialize animation mode
    unordered_map<unsigned int, vector<int>> animationModes;
	unsigned int animation_size = Serialization::deserializeAnimationMode(data, animationModes);
	for (auto p : animationModes) {
		models[p.first].model->movementMode = p.second[0]; // TODO: double check this (models)
		if(models[p.first].model->animationMode == -1 || p.second[1] != -1)
			models[p.first].model->animationMode = p.second[1];
	}
	data += animation_size;

	int currKill = leaderBoard->currentKills[player.player_id];

	// deserialize leaderboard
	unsigned int leaderBoard_size = 0;
	leaderBoard_size = Serialization::deserializeLeaderBoard(data, leaderBoard, killstreak_data);
	data += leaderBoard_size;

	if (leaderBoard->currentKills[player.player_id] > currKill) {
		audio.play(glm::vec3(0), CRACK_AUDIO);
		logger()->debug("you killed somebody");
	}
	while (!leaderBoard->kill_map.empty()) {
		int killer_id = leaderBoard->kill_map.front();
		leaderBoard->kill_map.pop_front();
		int dead_id = leaderBoard->kill_map.front();
		leaderBoard->kill_map.pop_front();

		string killername = usernames[killer_id];
		string deadname = usernames[dead_id];
		if (guiStatuses.killUpdates.size() < MAX_KILL_UPDATES) {
			guiStatuses.killUpdates.push_front(std::make_pair(killername, deadname));
		}
		else {
			guiStatuses.killUpdates.pop_back();
			guiStatuses.killUpdates.push_front(std::make_pair(killername, deadname));
		}
	}

	while (!leaderBoard->curr_killstreaks.empty()) {
		int killer_id = leaderBoard->curr_killstreaks.front();
		leaderBoard->curr_killstreaks.pop_front();
		int numStreaks = leaderBoard->curr_killstreaks.front();
		leaderBoard->curr_killstreaks.pop_front();

		string killername = usernames[killer_id];
		string update = killername + " is on KILLSTREAK " + (char)('0' +numStreaks) + "!";
		std::chrono::seconds s(TEXT_SHOW_TIME);
		std::chrono::nanoseconds ns(s);
		if (guiStatuses.killStreakUpdates.size() < MAX_KILL_UPDATES) {
			guiStatuses.killStreakUpdates.push_front(make_pair(update, ns));
		}
		else {
			guiStatuses.killStreakUpdates.pop_back();
			guiStatuses.killStreakUpdates.push_front(make_pair(update, ns));
		}
	}



	while (!leaderBoard->curr_shutdowns.empty()) {
		int killer_id = leaderBoard->curr_shutdowns.front();
		leaderBoard->curr_shutdowns.pop_front();
		int dead_id = leaderBoard->curr_shutdowns.front();
		leaderBoard->curr_shutdowns.pop_front();

		string killername = usernames[killer_id];
		string deadname = usernames[dead_id];
		string update = killername + " has shutdown " + deadname + "!";
		std::chrono::seconds s(TEXT_SHOW_TIME);
		std::chrono::nanoseconds ns(s);
		if (guiStatuses.killStreakUpdates.size() < MAX_KILL_UPDATES) {
			guiStatuses.killStreakUpdates.push_front(make_pair(update, ns));
		}
		else {
			guiStatuses.killStreakUpdates.pop_back();
			guiStatuses.killStreakUpdates.push_front(make_pair(update, ns));
		}
	}
	

	if (isCharging && (leaderBoard->currentKills[player.player_id] > currKill)) 
		skill_timers[DIR_SKILL_INDEX] = nanoseconds::zero();	// reset cooldown when kill someone using charge
   
	root = Serialization::deserializeSceneGraph(data, clientSceneGraphMap, particleTexture, particleShader);

	// nullify invisibility state if you're assassin (duh)
	if (player.modelType == ASSASSIN) {
		clientSceneGraphMap[player.root_id]->enabled = true;
	}

	glm::mat4 playerNode = clientSceneGraphMap[player.root_id]->M;
	//camera->cam_look_at = { playerNode[3][0],playerNode[3][1],playerNode[3][2] };
	//camera->cam_pos = initCamPos + glm::vec3({ playerNode[3][0],playerNode[3][1],playerNode[3][2] });
	glm::vec3 dest = { playerNode[3][0], playerNode[3][1], playerNode[3][2] };
	camera->setDestination(dest);
}


void ClientScene::setRoot(Transform * newRoot) {
	root = newRoot;
}

/*
	Get players gold.
*/
int ClientScene::getPlayerGold() 
{ 
	return player.gold;
}

/*
	Get players skills.
*/
vector<Skill> ClientScene::getPlayerSkills() 
{ 
	return player.availableSkills;
}

/*
	Get players usernames.
*/
vector<string> ClientScene::getUsernames() 
{ 
	return usernames;
}

bool ClientScene::checkInAnimation() {
	auto transform = clientSceneGraphMap[player.root_id];
	auto thisModel = models[*(transform->model_ids.begin())].model;
	return thisModel->curr_mode != thisModel->movementMode;
}

vector<int> ClientScene::getInvestmentInfo() {
	vector<int> result;
	result.push_back(player.amount_invested);
	result.push_back(player.player_invested_in);
	return result;
}

void ClientScene::clearInvestmentInfo() {
	player.amount_invested = 0;
	player.player_invested_in = NONE;
}

void ClientScene::updatePlayerGold(int curr_gold)
{
	player.gold = curr_gold;
}