#pragma once
#include <string>
#include <winsock2.h>
#include <Windows.h>
#include "NetworkServices.hpp"
#include "CoreTypes.hpp"
#include <ws2tcpip.h>
#include <map>

using namespace std;
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512 

class ServerNetwork
{
public:
	ServerNetwork(PCSTR host, PCSTR port);
	~ServerNetwork(void);

	// send data to all clients
	void broadcastSend(char * packets, int totalSize);

	// send data to one client
	void targetedSend(unsigned int client_id, char * packets, int totalSize);

	// receive incoming data
	int receiveData(unsigned int client_id, char * recvbuf);

	// accept new connections
	bool acceptNewClient(unsigned int & id);

	// close socket associated with client
	bool closeClientSocket(unsigned int id);

	// receive incoming data and deserialize into ClientInputPacket
	ClientInputPacket* receivePacket(unsigned int client_id);

	// deserialize client packet  
	ClientInputPacket* deserializeCP(char* temp_buff);

	// Socket to listen for new connections
	SOCKET listenSocket;

	// Socket to give to the clients
	SOCKET clientSocket;

	// for error checking return values
	int iResult;

	// table to keep track of each client's socket
	std::map<unsigned int, SOCKET> sessions;

protected:
	PCSTR serverPort;
};
