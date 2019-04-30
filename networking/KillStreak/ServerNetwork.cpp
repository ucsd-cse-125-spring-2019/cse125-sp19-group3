#include "ServerNetwork.hpp"
#include "Logger.hpp"

ServerNetwork::ServerNetwork(PCSTR host, PCSTR port) : serverPort(port)
{
	auto log = logger();
	// create WSADATA object
	WSADATA wsaData;

	// our sockets for the server
	listenSocket = INVALID_SOCKET;
	clientSocket = INVALID_SOCKET;

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
	iResult = getaddrinfo(host, serverPort, &hints, &result);

	if (iResult != 0) {
		log->error("getaddrinfo failed with error: {}", iResult);
		WSACleanup();
		exit(1);
	}

	// Create a SOCKET for connecting to server (create socket)
	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (listenSocket == INVALID_SOCKET) {
		log->error("socket failed with error: {}", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	// NOTE: Set it to blocking so 'accept' blocks until connection made
	// Set the mode of the socket to be nonblocking
	/*
	u_long iMode = 1;
	iResult = ioctlsocket(listenSocket, FIONBIO, &iMode);

	if (iResult == SOCKET_ERROR) {
		log->error("ioctlsocket failed with error: {}", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		exit(1);
	}
	*/

	// Setup the TCP listening socket (bind)
	const sockaddr* addr = result->ai_addr;
	int addrLen = (int)(result->ai_addrlen);
	iResult = bind(listenSocket, addr, addrLen);

	if (iResult == SOCKET_ERROR) {
		log->error("bind failed with error: {}", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		exit(1);
	}

	// no longer need address information
	freeaddrinfo(result);

	// start listening for new clients attempting to connect (listen)
	iResult = listen(listenSocket, SOMAXCONN);

	if (iResult == SOCKET_ERROR) {
		log->error("listen failed with error: {}", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		exit(1);
	}

	log->info("Server listening on {}:{}", host, port);
}


ServerNetwork::~ServerNetwork(void)
{
	// tear down all client / server sockets???
	closesocket(listenSocket);
	WSACleanup();
}


/* 
	Accept an incoming connection. 

	'id' will be client ID we want to associate with this socket.
	Check if client requested to initialize a connection as their first 
	request. Close connection if this was not their first request.
	Otherwise, nce socket is allocated, its mapped to the incoming clients ID
	in 'sessions' map.
*/
bool ServerNetwork::acceptNewClient(unsigned int & id)
{
	auto log = logger();

	// if client waiting, accept the connection and save the socket
	clientSocket = accept(listenSocket, NULL, NULL);

	if (clientSocket != INVALID_SOCKET)
	{
		//disable nagle on the client's socket
		char value = 1;
		setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

		// insert new client into session id table
		sessions.insert(pair<unsigned int, SOCKET>(id, clientSocket));

		// get clients initial request to join game
		ClientInputPacket* packet = receivePacket(id);
		if (!packet)		// error? 
		{
			closeClientSocket(clientSocket);
			return false;
		}

		// ensure client request is for initialization 
		if (packet->inputType != INIT_CONN)
		{
			log->info("MT: Client <{}>'s first request was not to initialize, closing connection.", id);
			closeClientSocket(clientSocket);
			return false;
		}

		log->info("MT: Connection accepted -> Client: {}", id);
		id++;		// inc to next client id

		return true;
	}

	log->info("MT: No connection: {}", WSAGetLastError());

	return false;
}


/*
	Close socket associated with client and remove from 'sessions' mapping.
	Return true if successfully closed, false otherwise.
*/
bool ServerNetwork::closeClientSocket(unsigned int id)
{
	auto log = logger();
	log->info("Closing client socket -> ID: {}", id);

	std::map<unsigned int, SOCKET>::iterator itr = sessions.begin();
	itr = sessions.find(id);
	if (itr != sessions.end())
	{
		SOCKET client_sock = itr->second;
		int iResult = closesocket(client_sock);
		if (iResult != 0) return false;
		return true;
	}

	log->error("Socket not found in mapping");
	return false;
}


/*
	Calls the receiveData function to receive a packet sized amount of data 
	from the client. On success will deserialize the data and return the packet. 

	-- Return deserialized packet from client, 0 on error
*/
ClientInputPacket* ServerNetwork::receivePacket(unsigned int client_id)
{

	// allocate buffer & receive data from client
	char *temp_buff = (char*)malloc(sizeof(ClientInputPacket));
	int bytes_read = receiveData(client_id, temp_buff);

	// client closed conection/error?
	if (!bytes_read || bytes_read == SOCKET_ERROR) return 0;

	// deserialize data into memory, point packet to it
	ClientInputPacket* packet = deserializeCP(temp_buff);
	return packet;
}


/* 
	Receive incoming data from client. Reads until packet size met. 
	All read data updates receiver buffer passed by reference.

	-- Returns amount of bytes read, 0 for closed connection, 
	SOCKET_ERROR otherwise. Updates 'recbuf' pointer passed by 
	reference with data read. 
*/
int ServerNetwork::receiveData(unsigned int client_id, char * recvbuf)
{
	auto log = logger();

	if (sessions.find(client_id) != sessions.end())		// find client 
	{
		// NOTE: For now, make all receives from client to server the same input packet type.

		SOCKET currentSocket = sessions[client_id];		// get client socket
		size_t toRead = sizeof(ClientInputPacket);		// amount of data to read
		char  *curr_bufptr = (char*) recvbuf;			// ptr to output buffer

		while (toRead > 0)								// read entire packet
		{
			auto rsz = recv(currentSocket, curr_bufptr, toRead, 0);

			if (rsz == 0) {
				log->error("CT <{}>: Client closed connection", client_id);
				closeClientSocket(client_id);
				return rsz;
			}

			if (rsz == SOCKET_ERROR) {
				log->error("CT <{}>: recv() failed {}", client_id, WSAGetLastError());
				closeClientSocket(client_id);
				return rsz;
			}

			toRead -= rsz;		 /* Read less next time */
			curr_bufptr += rsz;  /* Next buffer position to read into */
		}

		return sizeof(ClientInputPacket);
	}

	log->error("Receive error: Client mapping not found -> ID: {}", client_id);
	return 0;
}


// send data to all clients
void ServerNetwork::broadcastSend(ServerInputPacket packet)
{
	auto log = logger();
	SOCKET currentSocket;
	int iSendResult;

	char serialized[sizeof(ServerInputPacket)];
	memcpy(serialized, &packet, sizeof(ServerInputPacket));

	for (auto const& x : sessions)
	{
		currentSocket = x.second;
		int sentLength = send(currentSocket, serialized, sizeof(ServerInputPacket), 0);
		log->info("Sent packet of length {} to client {}", sentLength, x.first);
		//iSendResult = NetworkServices::sendMessage(currentSocket, packets, totalSize);

		//if (iSendResult == SOCKET_ERROR)
		//{
		//	log->error("send failed with error: {}", WSAGetLastError());
		//	closesocket(currentSocket);
		//}
	}
}


/*
	Serialize data and send to one client.
*/
int ServerNetwork::sendToClient(unsigned int client_id, ServerInputPacket packet)
{
	auto log = logger();

	if (sessions.find(client_id) != sessions.end())		// find client 
	{
		SOCKET currentSocket = sessions[client_id];		// get client socket
		char serialized[sizeof(ServerInputPacket)];
		memcpy(serialized, &packet, sizeof(ServerInputPacket));

		int sentLength = send(currentSocket, serialized, sizeof(ServerInputPacket), 0);
		return sentLength;
	}

	log->error("Receive error: Client mapping not found -> ID: {}", client_id);
	return 0;
}


/*
	Deserialize data sent from client back into a ClientInputPacket struct.
	-- Return reconstructed ClientInputPacket
*/
ClientInputPacket* ServerNetwork::deserializeCP(char* temp_buff)
{
	return reinterpret_cast<ClientInputPacket*>(temp_buff);
}


/*
	Initialize a packet to send to the client. 
*/
ServerInputPacket ServerNetwork::createServerPacket(ServerPacketType type, int size, char * data)
{
	ServerInputPacket packet;
	packet.packetType = type;
	packet.size = size;
	auto log = logger();
	log->info("Size of scene graph packet is {}", (size + 2));
	memcpy(packet.data, data, size);

	return packet;
}
