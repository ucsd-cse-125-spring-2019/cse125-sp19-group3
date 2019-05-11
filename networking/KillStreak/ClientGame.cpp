#include "ClientGame.h"
#include "ClientNetwork.hpp"
#include "Logger.hpp"
#include "sysexits.h"
#include <chrono>

#include "main.h"
#include "../../rendering/ClientScene.h"
#include "../../rendering/Serialization.h"


/*
	Contains pointer to network and queue that will store all 
	incoming packets from server.
*/
typedef struct {
	std::queue<ServerInputPacket*>* queuePtr;
	ClientNetwork* network;
	mutex* q_lock;
} server_data;


GLFWwindow * window = 0;


/*
	Constructor: parse config and initialize all data.
*/
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

	// get character selection time (config)
	char_select_time = (int) (config.GetInteger("game", "char_select_time", -1));
	if (char_select_time == -1) {
		log->error("Invalid char_select_time in config file");
		exit(EX_CONFIG);
	}

	q_lock = new mutex();
	serverPackets = new ServerInputQueue();
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
	// Set the scroll callback
	glfwSetScrollCallback(window, Window_static::scroll_callback);
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
	glClearColor(0.80f, 0.84f, 0.81f, 1.0f);
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


/*
	Launch new thread to keep an open recv() connection with the server. 
*/
void constantListenFromServer(void *arg)
{
	auto log = logger();
	server_data* server_arg = (server_data*) arg;
	mutex* q_lock = server_arg->q_lock;
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

		// TODO: REMOVE ME!!!
		//log->debug("--> Receiving packet from server size: {}", packet->size);
		//log->debug("--> Packet Type: {}", packet->packetType);

		// acquire queue lock & push packet 
		q_lock->lock();
		q_ptr->push(packet);			
		q_lock->unlock();

	} while (keep_conn);				// connection-closed/error? 

	network->closeSocket();				// close socket & free client_data 
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

	// send initial request to server (ask to join game, start my thread on the server!)
	ClientInputPacket init_packet = createInitPacket();
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


	// block until receive servers welcome package (telling us we're accepted and in lobby)
	ServerInputPacket* packet = network->receivePacket();
	if (!packet)									// error 
	{
		log->error("Error receiving servers initialization packet");
		return 0;
	}

	// a little bit hardcoded imo but ok
	if (packet->packetType != WELCOME)			// not initialization packet
	{
		log->error("Invalid initialization packet sent from server");
		free(packet);
		return 0;
	}
	handleServerInputPacket(packet);


	/* 
	CLIENT JOINED LOBBY ************************************************
		--> Block on recv() until server sends confirmation that all players joined 
			and its starting character selection 
	*/
	log->info("Received servers welcome packet, waiting in lobby for all players to join...");
	
	ServerInputPacket* char_select_packet = network->receivePacket();
	log->info("received character selection packet from server");

	handleServerInputPacket(char_select_packet);

	ServerInputPacket * initScenePacket = network->receivePacket();
	handleServerInputPacket(initScenePacket);


	// allocate new data for server thread & launch (will recv() indefinitely)
	server_data* server_arg = (server_data *)malloc(sizeof(server_data));
	if (server_arg)
	{
		server_arg->q_lock = q_lock;				// ptr to queue lock
		server_arg->queuePtr = serverPackets;		// ptr to server input queue
		server_arg->network = network;				// ptr to network
		_beginthread(constantListenFromServer, 0, (void*)server_arg);	
	}
	else	// error allocating server data; 
	{
		log->error("Error allocating server metadata");
		closesocket(network->ConnectSocket);
		return 0;
	}


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

	// GAME STARTING ******************************************************
	// This part will run after all players have selected their characters!

	// Create the GLFW window
	window = Window_static::create_window(640, 480);
	// Setup OpenGL settings, including lighting, materials, etc.
	setup_opengl_settings();
	// Print OpenGL and GLSL versions
	print_versions();
	// Setup callbacks
	setup_callbacks();
	// Initialize objects/pointers for rendering
	Window_static::initialize_objects(this, network);

	// Loop while GLFW window should stay open
	while (!glfwWindowShouldClose(window))
	{

		// TODO: REMOVE ME!!! (new thread should handle incoming packets
		ServerInputPacket* packet = network->receivePacket();
		handleServerInputPacket(packet);

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

int ClientGame::handleCharacterSelectionPacket(ServerInputPacket* packet) {
	auto log = logger();
	std::string username;			// user selected username
	ArcheType selected_type;		// user selected character

	// countdown until character selection phase is over
	auto Start = std::chrono::high_resolution_clock::now();
	while (1)
	{
		// time up? 
		auto End = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> Elapsed = End - Start;
		if (Elapsed.count() >= char_select_time)
			break;

		// TODO: Display UI (enter username, pick character) --> update values
		username = "Snake";		// TODO: REMOVE ME
		selected_type = HUMAN;	// TODO: REMOVE ME

	}

	log->info("Time up, sending selection data to server, waiting for game to start.");

	// create character selection packet & send to server
	ClientSelectionPacket selection_packet = createCharacterSelectedPacket(username, selected_type);
	int iResult = network->sendToServer(selection_packet);	// send

	// error?
	if (iResult != sizeof(ClientSelectionPacket))
	{
		log->error("Failure sending packet, only sent {} of {} bytes", iResult, sizeof(ClientSelectionPacket));
		return 0;
	}
	if (iResult == SOCKET_ERROR)
	{
		log->error("Send failed with error: {}", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	log->debug("Sent character data, waiting for game to start message from server!");
	return iResult;
}

ClientSelectionPacket ClientGame::createCharacterSelectedPacket(std::string username, ArcheType type) {
	ClientSelectionPacket packet;
	packet.username = username;
	packet.type = type;
	return packet;
}

ClientInputPacket ClientGame::createClientInputPacket(InputType type, Point finalLocation,
	int skillType, int attackType)
{
	ClientInputPacket packet;
	packet.inputType = type;
	packet.finalLocation = finalLocation;
	packet.skillType = skillType;
	packet.attackType = attackType;

	return packet;
}

ClientInputPacket ClientGame::createMovementPacket(Point newLocation) {
	return createClientInputPacket(MOVEMENT, newLocation, 0, 0);
}

ClientInputPacket ClientGame::createInitPacket() {
	return createClientInputPacket(INIT_CONN, NULL_POINT, 0, 0);
}

void ClientGame::handleServerInputPacket(ServerInputPacket * packet) {
	switch (packet->packetType) {
	case WELCOME:
		break;
	case CHAR_SELECT_PHASE:
		handleCharacterSelectionPacket(packet);
		break;
	case INIT_SCENE:
		Window_static::scene->handleInitScenePacket(packet->data);
		break;
	case UPDATE_SCENE_GRAPH:
		Window_static::scene->handleServerTickPacket(packet->data);
		break;
	default:
		break;
	}
	// deallocate the packet here
	free(packet);
}
