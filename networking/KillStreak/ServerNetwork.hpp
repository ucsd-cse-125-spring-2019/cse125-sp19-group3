#pragma once
#include <string>
#include <winsock2.h>
#include <Windows.h>
#include "NetworkServices.hpp"
#include "CoreTypes.hpp"
#include <ws2tcpip.h>
#include <map>
#include "../../rendering/Serialization.h"

using namespace std;
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512 

class ServerNetwork
{
public:
	ServerNetwork(PCSTR host, PCSTR port);
	~ServerNetwork(void);

	// send data to all clients
	void broadcastSend(ServerInputPacket packet);

	// serialize data & send to one client
	int sendToClient(unsigned int client_id, ServerInputPacket packet);

	// receive incoming data
	int receiveData(unsigned int client_id, char * recvbuf, size_t packet_size);

	// accept new connections
	bool acceptNewClient(unsigned int & id);

	// close socket associated with client
	bool closeClientSocket(unsigned int id);

	// create packet for server to send to client
	ServerInputPacket createServerPacket(ServerPacketType type, int temp, char * data);

	// receive incoming data and deserialize into ClientSelectionPacket
	ClientSelectionPacket* receiveSelectionPacket(unsigned int client_id);

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
