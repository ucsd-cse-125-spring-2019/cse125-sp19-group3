#pragma once
#include <winsock2.h>
#include "NetworkServices.hpp"
#include <ws2tcpip.h>
#include "Logger.hpp"
#include "CoreTypes.hpp"

#pragma comment (lib, "Ws2_32.lib")

class ClientNetwork {
public:
	int iResult;
	SOCKET ConnectSocket;

	ClientNetwork(PCSTR host, PCSTR serverPort);
	~ClientNetwork();

	// Serialize packet & send to server
	int sendToServer(ClientInputPacket packet);

	// initialize packet 
	ClientInputPacket createClientPacket(InputType type, Point finalLocation, 
		int skillType, int attackType);

	// receive data from server then deserialize into ServerInputPacket struct 
	ServerInputPacket* receivePacket();

	// receive data from server into buffer
	int receiveData(char* recvbuf);

	// deserialize data from server back into ServerInputPacket struct
	ServerInputPacket* deserializeSP(char* temp_buff);
};

