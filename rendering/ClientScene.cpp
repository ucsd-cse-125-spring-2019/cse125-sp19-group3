#include "ClientScene.h"

ClientScene * Window_static::scene = new ClientScene();

void ClientScene::initialize_objects(ClientGame * game)
{
	camera = new Camera();
	camera->SetAspect(width / height);
	camera->Reset();

	shader = new Shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);



	// TODO: add more models
	player_m = new Model(std::string("../BaseMesh_Anim.fbx"));

	//player = new Player(player_t, player_m);

	//cube = new Cube();
	//cube->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(0, 5, 10)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)); 
	//			// * glm::scale(glm::mat4(1.0f), glm::vec3(100, 0.01, 100)) * cube->toWorld;

	models.push_back(ModelData{player_m, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), shader, COLOR, 0});

	this->game = game;
}

void ClientScene::playerInit(const Player &player) {
	this->player = player;
	
	// TODO: move to here: 
	// 1) init player model
	// 2) create corresponding shader based on player data
	// 3) move camera
}

void ClientScene::clean_up()
{
	delete(camera);
	//delete(cube);
	delete(player_m);
	//delete(player_t);
	delete(root);
	delete(shader);
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
	camera->Update();
	player.update(time);
}

void ClientScene::display_callback(GLFWwindow* window)
{
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the shader of programID
	shader->use();
	
	// Render the cube
	//cube->draw(shader, glm::mat4(1.0f), camera->GetViewProjectMtx());

	// Now send these values to the shader program
	root->draw(shader, models, glm::mat4(1.0f), camera->GetViewProjectMtx());
	//models[0].model->draw(models[0].shader, player.playerRoot->M, camera->GetViewProjectMtx());
	//models[0].model->draw(models[0].shader, glm::mat4(1.0f), camera->GetViewProjectMtx());

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
	camera->cam_pos -= ((float)-yoffset * glm::normalize(z_dir));
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
		// I need to send the server a packet...
		game->sendPacket(MOVEMENT, new_dest, 0, 0);

		/*
		player->setDestination(new_dest);
		float dotResult = glm::dot(glm::normalize(new_dest - player->currentPos), player->currentOri);

		if (abs(dotResult) < 1.0) {
			float angle = glm::acos(dotResult);
			printf("rotate angle = %f", angle);
			glm::vec3 axis = glm::cross(player->currentOri, glm::normalize(new_dest - player->currentPos));
			if (glm::length(axis) != 0) {
				player->rotate(angle, axis);
			}
		}*/
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

char * ClientScene::deserializeInitScene(char * data, unsigned int size) {
	memcpy(&player.player_id, data, sizeof(unsigned int));
	data += sizeof(unsigned int);
	memcpy(&player.root_id, data, sizeof(unsigned int));
	data += sizeof(unsigned int);
	size -= 2 * sizeof(unsigned int);
	root = new Transform(glm::mat4(1.0f));
	char * retval = deserializeSceneGraph(data, size);
	for (auto child : root->children) {
		if (player.root_id == child.first) {
			this->player.playerRoot = child.second;
			break;
		}
	}
	return retval;
}

char * ClientScene::deserializeSceneGraph(char * data, unsigned int size) {
	char * retval = deserializeSceneGraph(root, data, size);
	return retval;
}

char * ClientScene::deserializeSceneGraph(Transform * t, char * data, unsigned int size) {
	memcpy(glm::value_ptr(t->M), data, sizeof(glm::mat4));
	data += sizeof(glm::mat4);
	size -= sizeof(glm::mat4);

	t->model_ids.clear();

	while (*data && size > 0) {
		char c = *data;
		data++;
		size -= sizeof(char);
		if (c == 'T') {
			unsigned int node_id;
			memcpy(&node_id, data, sizeof(unsigned int));
			data += sizeof(unsigned int);
			size -= sizeof(unsigned int);
			if (t->children.find(node_id) == t->children.end())
				t->addChild(node_id, new Transform());
			updated_ids.insert(node_id);
			data = deserializeSceneGraph(t->children[node_id], data, size);
		}
		else if (c == 'M') {
			unsigned int model_id;
			memcpy(&model_id, data, sizeof(unsigned int));
			data += sizeof(unsigned int);
			size -= sizeof(unsigned int);
			t->model_ids.insert(model_id);
		}
	}

	std::vector<unsigned int> to_remove;
	for (auto child : t->children) {
		if (updated_ids.find(child.first) == updated_ids.end()) {
			to_remove.push_back(child.first);
		}
	}
	for (unsigned int i : to_remove) {
		removeTransform(t, i);
	}
	return data;
}

void ClientScene::removeTransform(Transform * parent, const unsigned int node_id) {
	auto to_remove = parent->children[node_id];
	for (auto child : to_remove->children) {
		removeTransform(to_remove, child.first);
	}
	for (auto model_id : to_remove->model_ids) {
	}
	parent->removeChild(node_id);
}

