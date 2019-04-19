#pragma once
#include <winsock2.h>
#include "NetworkServices.hpp"
#include <ws2tcpip.h>
#include "Logger.hpp"

#pragma comment (lib, "Ws2_32.lib")

class ClientNetwork {
public:
	int iResult;
	SOCKET ConnectSocket;

	ClientNetwork(PCSTR host, PCSTR serverPort);
	~ClientNetwork();
};

