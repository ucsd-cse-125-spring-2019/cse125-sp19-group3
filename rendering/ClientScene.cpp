#include "ClientScene.h"
#include "nlohmann\json.hpp"
#include <fstream>

using json = nlohmann::json;

ClientScene * Window_static::scene = new ClientScene();

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
			models[(unsigned int)obj["model_id"]] = ModelData{ new Model(obj["path"],true), glm::vec4((float)(obj["color_rgb"][0]), (float)(obj["color_rgb"][1]), (float)(obj["color_rgb"][2]), 1.0f), animationShader, COLOR, 0 };
		}
		else {
			models[(unsigned int)obj["model_id"]] = ModelData{ new Model(obj["path"],false), glm::vec4((float)(obj["color_rgb"][0]), (float)(obj["color_rgb"][1]), (float)(obj["color_rgb"][2]), 1.0f), staticShader, COLOR, 0 };
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
}

void ClientScene::clean_up()
{
	delete(camera);
	//delete(cube);
	//delete(player_m);
	//delete(player_t);
	delete(root);
	delete(staticShader);
	delete(animationShader);
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
}

void ClientScene::idle_callback()
{
	// Call the update function the cube
	//cube->update();
	time += 1.0 / 60;
	glm::mat4 playerNode = clientSceneGraphMap[player.root_id]->M;
	camera->cam_look_at = { playerNode[3][0],playerNode[3][1],playerNode[3][2] };
	camera->cam_pos = initCamPos + glm::vec3({ playerNode[3][0],playerNode[3][1],playerNode[3][2] });
	camera->Update();
	for (auto &model : models) {
		if(model.second.model->isAnimated)
			model.second.model->BoneTransform("Take 001", time);
	}
}

void ClientScene::display_callback(GLFWwindow* window)
{
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the shader of programID
	//shader->use();
	
	auto vpMatrix = camera->GetViewProjectMtx();
	// Use the shader to draw all player + skill objects
	root->draw(models, glm::mat4(1.0f), vpMatrix, clientSceneGraphMap);

	// Use the shader to draw all environment objects
	for (auto &env_obj : env_objs) {
		env_obj->draw(models, glm::mat4(1.0f), vpMatrix, clientSceneGraphMap);
	}


	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();
	// Swap buffers
	glfwSwapBuffers(window);
}

void ClientScene::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Check for a key press
	if (action == GLFW_PRESS)
	{
		// Check if escape was pressed
		if (key == GLFW_KEY_ESCAPE)
		{
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);
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
		double xpos, ypos;
		//getting cursor position
		glfwGetCursorPos(window, &xpos, &ypos);
		printf("Cursor Position at %f: %f \n", xpos, ypos);
		glm::vec3 new_dest = viewToWorldCoordTransform(xpos, ypos);
		ClientInputPacket movementPacket = game->createMovementPacket(new_dest);
		network->sendToServer(movementPacket);
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


void ClientScene::handleInitScenePacket(char * data) {
	memcpy(&player.player_id, data, sizeof(unsigned int));
	data += sizeof(unsigned int);
	memcpy(&player.root_id, data, sizeof(unsigned int));
	data += sizeof(unsigned int);
	root = Serialization::deserializeSceneGraph(data, clientSceneGraphMap);
}

void ClientScene::handleServerTickPacket(char * data) {
	root = Serialization::deserializeSceneGraph(data, clientSceneGraphMap);
}


void ClientScene::setRoot(Transform * newRoot) {
	root = newRoot;
}

