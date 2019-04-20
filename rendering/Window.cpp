#include "window.h"

Window * Window_static::window = new Window();

void Window::initialize_objects()
{
	camera = new Camera();
	camera->SetAspect(width / height);
	camera->Reset();

	// Load the shader program. Make sure you have the correct filepath up top
	shaderProgram = new Shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);

	root = new Transform(glm::mat4(1.0f));
	
	cube = new Cube();
	cube->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(0, 5, 15)); //* glm::scale(glm::mat4(1.0f), glm::vec3(100, 0.01, 100)) * cube->toWorld;

	player = new Model(std::string("../BaseMesh_Anim.fbx"));

	models.push_back(ModelData{player, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), shaderProgram, COLOR, 0});
}

// Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::clean_up()
{
	delete(camera);
	delete(cube);
	delete(player);
	delete(root);
	delete(shaderProgram);
	//glDeleteProgram(shaderProgram);
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
	Window::resize_callback(window, width, height);

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
}

void Window::display_callback(GLFWwindow* window)
{
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the shader of programID
	shaderProgram->use();
	
	// Render the cube
	//cube->draw(shaderProgram);

	glm::mat4 tw = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)) 
		* glm::rotate(glm::mat4(1.0f), -90 / 180.0f * glm::pi<float>(), glm::vec3(1, 0, 0)) 
		* glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
	glm::mat4 MVP = camera->GetViewProjectMtx() * tw;
	// Now send these values to the shader program
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &tw[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelViewProjection"), 1, GL_FALSE, &MVP[0][0]);
	model->Draw(shaderProgram);

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
