#include "ServerNetwork.hpp"
#include "Logger.hpp"

ServerNetwork::ServerNetwork(PCSTR port) : serverPort(port)
{
	auto log = logger();
	// create WSADATA object
	WSADATA wsaData;

	// our sockets for the server
	ListenSocket = INVALID_SOCKET;
	ClientSocket = INVALID_SOCKET;

	// address info for the server to listen to
	struct addrinfo *result = NULL;
	struct addrinfo hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		log->error("WSAStartup failed with error: {}", iResult);
		exit(1);
	}

	// set address information
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;    // TCP connection!!!
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, serverPort, &hints, &result);

	if (iResult != 0) {
		log->error("getaddrinfo failed with error: {}", iResult);
		WSACleanup();
		exit(1);
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		log->error("socket failed with error: {}", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	// Set the mode of the socket to be nonblocking
	u_long iMode = 1;
	iResult = ioctlsocket(ListenSocket, FIONBIO, &iMode);

	if (iResult == SOCKET_ERROR) {
		log->error("ioctlsocket failed with error: {}", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	// Setup the TCP listening socket
	const sockaddr* addr = result->ai_addr;
	int addrLen = (int)(result->ai_addrlen);
	iResult = bind(ListenSocket, addr, addrLen);

	if (iResult == SOCKET_ERROR) {
		log->error("bind failed with error: {}", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	// no longer need address information
	freeaddrinfo(result);

	// start listening for new clients attempting to connect
	iResult = listen(ListenSocket, SOMAXCONN);

	if (iResult == SOCKET_ERROR) {
		log->error("listen failed with error: {}", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
}


ServerNetwork::~ServerNetwork(void)
{
	// tear down all client / server sockets???
	closesocket(ListenSocket);
	WSACleanup();
}

// accept new connections
bool ServerNetwork::acceptNewClient(unsigned int & id)
{
	// if client waiting, accept the connection and save the socket
	ClientSocket = accept(ListenSocket, NULL, NULL);

	if (ClientSocket != INVALID_SOCKET)
	{
		//disable nagle on the client's socket
		char value = 1;
		setsockopt(ClientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

		// insert new client into session id table
		sessions.insert(pair<unsigned int, SOCKET>(id, ClientSocket));

		return true;
	}

	// TODO: Should we send initial state data to connected client here? 

	return false;
}


// TODO: Decide on which layer to put Serializer and Deserializer (in Network or in game??)


// receive incoming data
int ServerNetwork::receiveData(unsigned int client_id, char * recvbuf)
{
	auto log = logger();
	if (sessions.find(client_id) != sessions.end())
	{
		SOCKET currentSocket = sessions[client_id];
		iResult = NetworkServices::receiveMessage(currentSocket, recvbuf, MAX_PACKET_SIZE);

		if (iResult == 0)
		{
			log->error("Connection closed");
			closesocket(currentSocket);
		}

		return iResult;
	}

	return 0;
}

// send data to all clients
void ServerNetwork::broadcastSend(char * packets, int totalSize)
{
	auto log = logger();
	SOCKET currentSocket;
	int iSendResult;

	for (auto const& x : sessions)
	{
		currentSocket = x.second;
		iSendResult = NetworkServices::sendMessage(currentSocket, packets, totalSize);

		if (iSendResult == SOCKET_ERROR)
		{
			log->error("send failed with error: {}", WSAGetLastError());
			closesocket(currentSocket);
		}
	}
}

// send data to one client
void ServerNetwork::targetedSend(unsigned int client_id, char * packets, int totalSize)
{
	SOCKET currentSocket;
	int iSendResult;

	currentSocket = sessions[client_id];
	iSendResult = NetworkServices::sendMessage(currentSocket, packets, totalSize);

	if (iSendResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(currentSocket);
	}
}