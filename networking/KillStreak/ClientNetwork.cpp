#include "ClientNetwork.hpp"
#include "Logger.hpp"


ClientNetwork::ClientNetwork(PCSTR host, PCSTR serverPort) {
	auto log = logger();

	// create WSDATA object
	WSADATA wsaData;

	// socket
	ConnectSocket = INVALID_SOCKET;

	// holds address info for socket to connect to
	struct addrinfo* result = NULL, *ptr = NULL, hints;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0) {
		log->error("WSAStartup failed with error: {}", iResult);
		exit(1);
	}

	// set address info
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; // TCP baby

	// resole server address and port
	iResult = getaddrinfo(host, serverPort, &hints, &result);

	if (iResult != 0) {
		log->error("getaddrinfo failed with error: {}", iResult);
		WSACleanup();
		exit(1);
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			log->error("socket failed with error: {}", WSAGetLastError());
			WSACleanup();
			exit(1);
		}

		// Connect to server
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)(ptr->ai_addrlen));
		if (iResult == SOCKET_ERROR) {
			log->error("The server is down... did not connect: {}", WSAGetLastError());
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
		}
	}

	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		log->error("Unable to connect to server!");
		WSACleanup();
		exit(1);
	}

	// TODO: Should client block until connection is accepted? 

	log->info("Client connected on {}:{}", host, serverPort);

	/*
	// TODO: Do we want this to block? 
	// set the mode of the socket to be nonblocking
	u_long iMode = 1;
	iResult = ioctlsocket(ConnectSocket, FIONBIO, &iMode);
	if (iResult == SOCKET_ERROR) {
		log->error("ioctlsocket failed with error: {}", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		exit(1);
	}
	*/

	// disable nagle (??)
	// TODO: TCP optimization? 
	// https://www.extrahop.com/company/blog/2016/tcp-nodelay-nagle-quickack-best-practices/
	char value = 1;
	setsockopt(ConnectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
}


ClientNetwork::~ClientNetwork(void) {

}


/*
	Serialize packet then send to server.
*/
int ClientNetwork::sendToServer(ClientInputPacket packet) {
	char serialized[sizeof(ClientInputPacket)];
	memcpy(serialized, &packet, sizeof(ClientInputPacket));

	int sentLength = send(ConnectSocket, serialized, sizeof(ClientInputPacket), 0);
	return sentLength;
}