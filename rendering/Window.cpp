#include "window.h"

Window * Window_static::window = new Window();

void Window::initialize_objects()
{
	camera = new Camera();
	camera->SetAspect(width / height);
	camera->Reset();

	// Load the shader program. Make sure you have the correct filepath up top
	shader = new Shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);

	root = new Transform(glm::mat4(1.0f));
	
	player_t = new Transform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0))
		* glm::rotate(glm::mat4(1.0f), -90 / 180.0f * glm::pi<float>(), glm::vec3(1, 0, 0))
		* glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)));
	player_t->model_ids.insert(PLAYER);
	root->addChild(1, player_t);

	player_m = new Model(std::string("../BaseMesh_Anim.fbx"));

	player = new Player(player_t, player_m);

	cube = new Cube();
	cube->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(0, 5, 10)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)); 
				// * glm::scale(glm::mat4(1.0f), glm::vec3(100, 0.01, 100)) * cube->toWorld;

	models.push_back(ModelData{player_m, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), shader, COLOR, 0});
}

void Window::clean_up()
{
	delete(camera);
	delete(cube);
	delete(player_m);
	delete(player_t);
	delete(root);
	//glDeleteProgram(shader);
}

GLFWwindow* Window::create_window(int width, int height)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__ // Because Apple hates comforming to standards
	// Ensure that minimum OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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
	//Window::resize_callback(window, width, height);
	Window::width = width;
	Window::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	return window;
}

void Window::resize_callback(GLFWwindow* window, int width, int height)
{
#ifdef __APPLE__
	glfwGetFramebufferSize(window, &width, &height); // In case your Mac has a retina display
#endif
	Window::width = width;
	Window::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		camera->SetAspect((float)width / (float)height);
	}
}

void Window::idle_callback()
{
	// Call the update function the cube
	//cube->update();
	camera->Update();
	player->update();
}

void Window::display_callback(GLFWwindow* window)
{
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the shader of programID
	shader->use();
	
	// Render the cube
	cube->draw(shader, glm::mat4(1.0f), camera->GetViewProjectMtx());

	// Now send these values to the shader program
	root->draw(shader, models, glm::mat4(1.0f), camera->GetViewProjectMtx());

	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();
	// Swap buffers
	glfwSwapBuffers(window);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
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

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	glm::vec3 z_dir = camera->cam_look_at - camera->cam_pos;
	camera->cam_pos -= ((float)-yoffset * glm::normalize(z_dir));
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		double xpos, ypos;
		//getting cursor position
		glfwGetCursorPos(window, &xpos, &ypos);
		printf("Cursor Position at %f: %f \n", xpos, ypos);
		glm::vec3 new_dest = viewToWorldCoordTransform(xpos, ypos);
		player->setDestination(new_dest);
		float angle = glm::acos(glm::dot(glm::normalize(new_dest - player->currentPos), player->currentOri));
		printf("rotate angle = %f", angle);
		glm::vec3 axis = glm::cross(player->currentOri, glm::normalize(new_dest - player->currentPos));
		//player->rotate(angle, axis);

		//std::cout << "Cursor Position at (" << xpos << " : " << ypos << std::endl;
	}
}

// SCREEN SPACE: mouse_x and mouse_y are screen space
glm::vec3 Window::viewToWorldCoordTransform(int mouse_x, int mouse_y) {
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
	printf("world pos map to: %f %f %f\n", worldPos.x, worldPos.y, worldPos.z);
	glm::vec3 realPos = glm::vec3(worldPos.x / worldPos.w, worldPos.y / worldPos.w, worldPos.z / worldPos.w);
	

	glm::vec3 cam_pos = camera->cam_pos;
	glm::vec3 dir = glm::normalize(realPos - cam_pos);
	float n = -cam_pos.y / dir.y;
	realPos.x = cam_pos.x + n * dir.x;
	realPos.y = 0;
	realPos.z = cam_pos.z + n * dir.z;

	printf("world pos remap to: %f %f %f\n", realPos.x, realPos.y, realPos.z);

	return realPos;
}
