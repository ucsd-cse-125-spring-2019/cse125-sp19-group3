#pragma once
#include <string>
#include <winsock2.h>
#include <Windows.h>
#include "NetworkServices.hpp"
#include <ws2tcpip.h>
#include <map>
// #include "NetworkData.h"
using namespace std;
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512 

class ServerNetwork
{
public:
	ServerNetwork(PCSTR port);
	~ServerNetwork(void);

	// send data to all clients
	void broadcastSend(char * packets, int totalSize);
	// send data to one client
	void targetedSend(unsigned int client_id, char * packets, int totalSize);

	// receive incoming data
	int receiveData(unsigned int client_id, char * recvbuf);

	// accept new connections
	bool acceptNewClient(unsigned int & id);

	// Socket to listen for new connections
	SOCKET ListenSocket;

	// Socket to give to the clients
	SOCKET ClientSocket;

	// for error checking return values
	int iResult;

	// table to keep track of each client's socket
	std::map<unsigned int, SOCKET> sessions;

protected:
	PCSTR serverPort;
};
