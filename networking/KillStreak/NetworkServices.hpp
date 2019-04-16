#pragma once
#include <winsock2.h>
#include <Windows.h>

#define MAX_PACKET_SIZE 1000000
class NetworkServices
{
public:
	static int sendMessage(SOCKET curSocket, char * message, int messageSize);
	static int receiveMessage(SOCKET curSocket, char * buffer, int bufSize);
};