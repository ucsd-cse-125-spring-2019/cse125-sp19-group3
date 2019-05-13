#include "main.h"
#include "nlohmann\json.hpp"
#include <stdlib.h>
#include <fstream>
#include <string>

#define SERVER_CONF "../../networking/KillStreak/server_config.json"
#define CLIENT_CONF "../../networking/KillStreak/client_config.json"

using json = nlohmann::json;
using namespace std;

ServerGame * server = nullptr;
ClientGame * client = nullptr;


// launch initialized server (called by new thread)
void run_server_thread(void *arg)
{
	server->launch();
}


// initialize and run server
void init_server(int multi=0)
{
	// parse config
	ifstream json_file(SERVER_CONF);
	json jsonObjs = json::parse(json_file);
	auto obj = jsonObjs["server"];

	// get host & port
	string host = obj["host"];
	string port = obj["port"];

	// get tick_rate & convert to double
	string str_tick_rate = obj["tick_rate"];
	double tick_rate = atof(str_tick_rate.c_str());

	logger()->info("Launching Killstreak server");
	server = new ServerGame(host, port, tick_rate);

	if (multi == 0)			// non multi-threaded	
	{
		server->launch();
	}
	else					// multi-threaded
	{
		_beginthread(run_server_thread, 0, 0);
	}

}


// initialize and run client
void init_client()
{

	// parse config
	ifstream json_file(CLIENT_CONF);
	json jsonObjs = json::parse(json_file);
	auto obj = jsonObjs["client"];

	// get host & port
	string host = obj["host"];
	string port = obj["port"];

	// get character selection time
	obj = jsonObjs["game"];
	string str_cst = obj["char_select_time"];
	int char_select_time = stoi(str_cst);		// convert to int

	logger()->info("Launching Killstreak client");
	client = new ClientGame(host, port, char_select_time);
	client->run();

}


int main(int argc, char** argv) {
	initLogging();
	auto log = logger();

	if (argc != 2)
	{
		cerr << "Invalid number of arguments" << endl;
		return EX_USAGE;
	}


	// NOTE: Once in production can make single config file for both server & client
	// Can then use 'enabled' to check if we should run server or client. 

	std::string arg = argv[1];
	if (arg == "server")	  // run the server
	{
		init_server();
	}
	else if (arg == "client") // run the client
	{
		init_client();
	}
	else if (arg == "multi") // run both server and single client
	{
		init_server(1);
		init_client();
	}
	else
	{
		log->error("Invalid arg: Must enter 'server', 'client', or 'multi'");
	}


	return 0;
}
