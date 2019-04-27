#include "ClientGame.h"
#include "ClientNetwork.hpp"
#include "Logger.hpp"
#include "sysexits.h"

#include "../../rendering/main.h"

typedef struct {
	std::queue<ServerInputPacket>* queuePtr;
	ClientNetwork* network;
} server_data;

GLFWwindow * window = 0;

ClientGame::ClientGame(INIReader& t_config) : config(t_config) {
	auto log = logger();

	// get client data from config file
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
	glfwSetKeyCallback(window, Window_static::key_callback);
	// Set the mouse callback
	glfwSetMouseButtonCallback(window, Window_static::mouse_button_callback);
	// Set the window resize callback
	glfwSetFramebufferSizeCallback(window, Window_static::resize_callback);
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
	glClearColor(0.05f, 0.35f, 0.05f, 1.0f);
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

void constantListenFromServer(void *arg)
{
	auto log = logger();
	server_data* server_arg = (server_data*) arg;
	ServerInputQueue* q_ptr = server_arg->queuePtr;
	ClientNetwork* network = server_arg->network;

	log->info("Launching thread on client to listen to server");
	int keep_conn = 1;					// keep connection alive
	do {

		// get packet from client
		ServerInputPacket* packet = network->receivePacket();
		if (!packet) {
			keep_conn = 0;
			break;
		}

		q_ptr->push(*packet);		// push packet to queue

	} while (keep_conn);				// connection-closed/error? 


										// close socket & free client_data 
	network->closeSocket();
	free(server_arg);
}


/*
	Send initial request to server asking to join game. Will hang until request accepted 
	by server and initial data is sent back to client.
	Once client gets pre-game metadata they will remain in the lobby until game start.

	-- Return 1 on success, 0 on failure
*/
int ClientGame::join_game()
{
	auto log = logger();

	log->info("Sending initialization packet...");
	// send initial request to server 
	ClientInputPacket init_packet = network->createClientPacket(INIT_CONN, NULL_POINT, 0, 0);
	int iResult = network->sendToServer(init_packet);

	// error?
	if (iResult != sizeof(ClientInputPacket))
	{
		log->error("Failure sending packet, only sent {} of {} bytes", iResult, sizeof(ClientInputPacket));
		return 0;
	} 
	if (iResult == SOCKET_ERROR)
	{
		log->error("Send failed with error: {}", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	// block until receive servers init package
	ServerInputPacket* packet = network->receivePacket();
	if (!packet)								// error 
	{
		log->error("Error receiving servers initialization packet");
		return 0;
	}
	if (packet->packetType != INIT_SCENE)			// not initialization packet
	{
		log->error("Invalid initialization packet send from server");
		return 0;
	}


	server_data* server_arg = (server_data *)malloc(sizeof(server_data));

	/*server_arg->queuePtr = &serverPackets;
	server_arg->network = network;
	_beginthread(constantListenFromServer, 0, (void*)network);*/


	log->debug("DATA FROM SERVER: {} {}", packet->packetType, packet->size);


	// client successfully received servers initialization packet with the
	// pre-game metadata and is now in the lobby
	// TODO:: CLIENT LOBBY !!!
	log->info("Received servers init package, waiting in lobby!!");
	//while (1);


	return 1;
}


void ClientGame::run() {
	auto log = logger();
	log->info("Client running...");

	// attempt to join game
	int iResult = join_game();
	if (!iResult)
	{
		log->error("Closing connection");
		closesocket(network->ConnectSocket);
		return;
	}


	// This part will run after all players join and players leave lobby!
	// GAME STARTING ******************************************************

	//while (1) {};	// TODO: REMOVE ME!
	
	// TODO (MAYBE?): Put timeout on client socket, if server game full will disconnect? 

	
	// TEST: Sending initial message, does server recv()? 
	log->info("Client: Sending message...");
	Point finalLocation = Point(1.0, 2.0, 3.0);
	ClientInputPacket testPacket = network->createClientPacket(MOVEMENT, finalLocation, 0, 0);
	ClientInputPacket testPacket2 = network->createClientPacket(MOVEMENT, finalLocation, 0, 0);

	iResult = network->sendToServer(testPacket);
	iResult = network->sendToServer(testPacket2);
	
	if (iResult == SOCKET_ERROR) {
		wprintf(L"send failed with error: %d\n", WSAGetLastError());
		closesocket(network->ConnectSocket);
		WSACleanup();
		return;
	}

	log->info("Client: Bytes sent: {}", iResult);


	// GRAPHICS CODE *******************************************************

	// Create the GLFW window
	window = Window_static::create_window(640, 480);
	// Setup OpenGL settings, including lighting, materials, etc.
	setup_opengl_settings();
	// Print OpenGL and GLSL versions
	print_versions();
	// Setup callbacks
	setup_callbacks();
	// Initialize objects/pointers for rendering
	Window_static::initialize_objects(this);

	// Loop while GLFW window should stay open
	while (!glfwWindowShouldClose(window))
	{
		/*
		1) grab a packet from the serverInputQueue
		2) render scene based off of packet + update any game state UI / variable based on packet.
		*/

		/*if (!(serverPackets.empty())) {
			ServerInputPacket packet = serverPackets.front();
			serverPackets.pop();
			Window_static::window->deserializeSceneGraph(packet.data, packet.size);
		}*/

		ServerInputPacket* packet = network->receivePacket();
		log->info("client received packet of size {}", packet->size);
		Window_static::window->deserializeSceneGraph(packet->data, packet->size);


		// Main render display callback. Rendering of objects is done here.
		Window_static::display_callback(window);
		// Idle callback. Updating objects, etc. can be done here.
		Window_static::idle_callback();
	}

	Window_static::clean_up();
	// Destroy the window
	glfwDestroyWindow(window);
	// Terminate GLFW
	glfwTerminate();

	exit(EXIT_SUCCESS);
	
}

void ClientGame::sendPacket(InputType inputType, Point finalLocation, int skillType, int attackType) {
	ClientInputPacket packet = network->createClientPacket(inputType, finalLocation, skillType, attackType);
	network->sendToServer(packet);
}
