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


	// TODO: TCP optimization? 
	// https://www.extrahop.com/company/blog/2016/tcp-nodelay-nagle-quickack-best-practices/
	char value = 1;
	setsockopt(ConnectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
}


ClientNetwork::~ClientNetwork(void) {

}

void ClientNetwork::closeSocket() {
	closesocket(ConnectSocket);
}


/*
	Serialize a packet then send to server.
*/
int ClientNetwork::sendToServer(ClientInputPacket packet) {
	char serialized[sizeof(ClientInputPacket)];
	memcpy(serialized, &packet, sizeof(ClientInputPacket));

	int sentLength = send(ConnectSocket, serialized, sizeof(ClientInputPacket), 0);
	return sentLength;
}


/*
	Initialize a packet to send to the server. 
*/
ClientInputPacket ClientNetwork::createClientPacket(InputType type, Point finalLocation,
	int skillType, int attackType)
{
	ClientInputPacket packet;
	packet.inputType = type;
	packet.finalLocation = finalLocation;
	packet.skillType = skillType;
	packet.attackType = attackType;

	return packet;
}


/*
	Calls the receiveData function to receive a packet sized amount of data 
	from the client. On success will deserialize the data and return the packet. 

	-- Return deserialized packet from server, 0 on error
*/
ServerInputPacket* ClientNetwork::receivePacket()
{

	// allocate buffer & receive data from client
	char* temp_buff = (char*)malloc(sizeof(ServerInputPacket));
	int bytes_read = receiveData(temp_buff);

	logger()->debug("Pre-deserialization packet size: {}", bytes_read);

	// client closed conection/error?
	if (!bytes_read || bytes_read == SOCKET_ERROR) return 0;

	// deserialize data into memory, point packet to it
	ServerInputPacket* packet = deserializeSP(temp_buff);
	logger()->debug("Deserialized packet size: {}", bytes_read);
	return packet;
}


/* 
	Receive incoming data from server. Reads until packet size met. 
	All read data updates receiver buffer passed by reference.

	-- Returns amount of bytes read, 0 for closed connection, 
	SOCKET_ERROR otherwise. Updates 'recbuf' pointer passed by 
	reference with data read. 
*/
int ClientNetwork::receiveData(char * recvbuf)
{
	auto log = logger();

	SOCKET currentSocket = ConnectSocket;			// get client socket
	size_t toRead = sizeof(ServerInputPacket);		// amount of data to read
	char  *curr_bufptr = (char*) recvbuf;			// ptr to output buffer

	while (toRead > 0)								// read entire packet
	{
		auto rsz = recv(currentSocket, curr_bufptr, toRead, 0);

		if (rsz == 0) {
			log->error("Server closed connection");
			closesocket(ConnectSocket);
			return rsz;
		}

		if (rsz == SOCKET_ERROR) {
			log->error("recv() failed {}", WSAGetLastError());
			closesocket(ConnectSocket);
			return rsz;
		}

		toRead -= rsz;		 /* Read less next time */
		curr_bufptr += rsz;  /* Next buffer position to read into */
	}

	return sizeof(ServerInputPacket);

}


/*
	Deserialize data sent from server back into a ServerInputPacket struct.
	-- Return reconstructed ServerInputPacket
*/
ServerInputPacket* ClientNetwork::deserializeSP(char* temp_buff)
{
	return reinterpret_cast<ServerInputPacket*>(temp_buff);
}
