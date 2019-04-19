#include "ClientGame.h"
#include "ClientNetwork.hpp"
#include "Logger.hpp"
#include "sysexits.h"

#include "../../rendering/main.h"

GLFWwindow* window;

ClientGame::ClientGame(INIReader& t_config) : config(t_config) {
	auto log = logger();
	// PULL WHATEVER YOU NEED FROM THE CONFIG FILE HERE...
	string servconf = config.Get("client", "host", "");
	if (servconf == "") {
		log->error("Host line not found in config file");
		exit(EX_CONFIG);
	}
	size_t idx = servconf.find(":");
	if (idx == string::npos) {
		log->error("Config line {} is invalid", servconf);
		exit(EX_CONFIG);
	}

	// get host (config)
	string get_host = servconf.substr(0, idx);
	host = get_host.c_str();

	// get port (config)
	string get_port = servconf.substr(idx + 1);
	serverPort = get_port.c_str();
	
	network = new ClientNetwork(host, serverPort);
}



void error_callback(int error, const char* description)
{
	// Print error
	fputs(description, stderr);
}

void setup_callbacks()
{
	// Set the error callback
	glfwSetErrorCallback(error_callback);
	// Set the key callback
	glfwSetKeyCallback(window, Window::key_callback);
	// Set the window resize callback
	glfwSetFramebufferSizeCallback(window, Window::resize_callback);
}

void setup_glew()
{
	// Initialize GLEW. Not needed on OSX systems.
#ifndef __APPLE__
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		glfwTerminate();
	}
	fprintf(stdout, "Current GLEW version: %s\n", glewGetString(GLEW_VERSION));
#endif
}

void setup_opengl_settings()
{
#ifndef __APPLE__
	// Setup GLEW. Don't do this on OSX systems.
	setup_glew();
#endif
	// Enable depth buffering
	glEnable(GL_DEPTH_TEST);
	// Related to shaders and z value comparisons for the depth buffer
	glDepthFunc(GL_LEQUAL);
	// Set polygon drawing mode to fill front and back of each polygon
	// You can also use the paramter of GL_LINE instead of GL_FILL to see wireframes
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// Disable backface culling to render both sides of polygons
	glDisable(GL_CULL_FACE);
	// Set clear color
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
}

void print_versions()
{
	// Get info of GPU and supported OpenGL version
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

	//If the shading language symbol is defined
#ifdef GL_SHADING_LANGUAGE_VERSION
	std::printf("Supported GLSL version is %s.\n", (char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif
}

void ClientGame::run() {
	auto log = logger();

	// Create the GLFW window
	window = Window::create_window(640, 480);
	// Print OpenGL and GLSL versions
	print_versions();
	// Setup callbacks
	setup_callbacks();
	// Setup OpenGL settings, including lighting, materials, etc.
	setup_opengl_settings();
	// Initialize objects/pointers for rendering
	Window::initialize_objects();

	log->info("Client running...");
	// Loop while GLFW window should stay open
	while (!glfwWindowShouldClose(window))
	{
		// Main render display callback. Rendering of objects is done here.
		Window::display_callback(window);
		// Idle callback. Updating objects, etc. can be done here.
		Window::idle_callback();
	}

	Window::clean_up();
	// Destroy the window
	glfwDestroyWindow(window);
	// Terminate GLFW
	glfwTerminate();

	exit(EXIT_SUCCESS);
}