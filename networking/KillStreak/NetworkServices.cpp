#include "NetworkServices.hpp"

int NetworkServices::sendMessage(SOCKET curSocket, char * message, int messageSize)
{
	return send(curSocket, message, messageSize, 0); // how does it know which send / recv function to use?
}

int NetworkServices::receiveMessage(SOCKET curSocket, char * buffer, int bufSize)
{
	return recv(curSocket, buffer, bufSize, 0);
}