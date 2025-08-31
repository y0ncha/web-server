#include "server.h"

Server::Server(const std::string& ip, int port, std::size_t buffer_size)
    : ip_(ip), port_(port), recv_buffer_size_(buffer_size)
{
	// Initialize Winsock, if fails set listenSocket to INVALID_SOCKET and return
    WSAData wsaData;
    if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        reportError("Error at WSAStartup()", 0);
        listenSocket = INVALID_SOCKET;
        return;
    }
	// Create a listening socket, if fails set listenSocket to INVALID_SOCKET and return
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == listenSocket) {
        reportError("Error at socket()", 1);
        return;
    }
	// Set up the sockaddr_in structure
    sockaddr_in serverService;
	serverService.sin_family = AF_INET; // IPv4
	serverService.sin_addr.s_addr = inet_addr(ip_.c_str()); // Use given IP address
	serverService.sin_port = htons(port_); // Use given port

    // Bind the listening socket, if fails close socket, set to INVALID_SOCKET and return
    if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService))) {
        reportError("Error at bind()", 1);
        return; 
    }
}

Server::~Server() {

    // Close the listening socket
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
    }
    // Cleanup Winsock
    WSACleanup();
    std::cout << "Web Server: Closing Connection.\n";
}

void Server::reportError(const std::string& msg, bool cleanupWSA) {
    std::cerr << "Web Server: " << msg << " Error: " << WSAGetLastError() << std::endl;
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
    if (cleanupWSA) {
        WSACleanup();
    }
}

bool Server::listen() {
	// Start listening for incoming connection requests
    if (SOCKET_ERROR == ::listen(listenSocket, 5)) {
        reportError("Error at listen()", true);
        return false;
    }
    unsigned long flag = 1;
	// Set the listening socket to be non-blocking
    if (ioctlsocket(listenSocket, FIONBIO, &flag) != NO_ERROR) {
        reportError("Error at ioctlsocket()", true);
        return false;
    }
    return true;
}

bool Server::addClient(SOCKET clientSocket, const sockaddr_in& addr) {

    unsigned long flag = 1;
	// Set the client socket to be non-blocking
    if (ioctlsocket(clientSocket, FIONBIO, &flag) != NO_ERROR) {
        closesocket(clientSocket);
        return false;
    }
	// Emplace a new client into the clients map
    auto result = clients.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(clientSocket),
        std::forward_as_tuple(clientSocket, addr)
    );
	// If insertion failed, close the socket and return false
    if (!result.second) {
        closesocket(clientSocket);
        return false;
    }
    return true;
}

void Server::acceptConnection() {
    sockaddr_in from;
    int fromLen = sizeof(from);
    SOCKET msgSocket = accept(listenSocket, (sockaddr*)&from, &fromLen);

    // If accept failed, report the error and return
    if (INVALID_SOCKET == msgSocket) {
        reportError("Error at accept()", 0);
        return;
    }
    // Add the new client to the clients map, if fails close the socket and return
    if (addClient(msgSocket, from)) {
		clients[msgSocket].setAwaitingData(); // Start in AwaitingData state (Even though constructor does this too)
    }
}

void Server::receiveMessage(Client& client) {
    std::string tempBuff(recv_buffer_size_, '\0');
    int bytesRecv = recv(client.socket, &tempBuff[0], static_cast<int>(tempBuff.size() - 1), 0);

    // If recv failed, report the error, close the socket and remove the client
    if (SOCKET_ERROR == bytesRecv) {
        reportError("Error at recv()", false);
        client.setTerminated();
        closesocket(client.socket);
        clients.erase(client.socket);
        return;
    }
    // If connection was closed by client, close the socket and remove the client
    if (bytesRecv == 0) {
		client.setFinished();
        closesocket(client.socket);
        clients.erase(client.socket);
        return;
    }
    tempBuff.resize(bytesRecv);

    client.setRequestBuffered(tempBuff); // Buffer the request at client, updates last_active and state
    std::cout << "Web Server: Received: " << bytesRecv << " bytes of \"" << tempBuff << "\" message.\n";
}

void Server::sendMessage(Client& client) {

	// If not in Transmitting state or out_buffer is empty, nothing to send
    if (client.state != ClientState::Transmitting || client.out_buffer.empty()) {
        return;
    }
    int bytesSent = send(client.socket, client.out_buffer.c_str(), (int)client.out_buffer.size(), 0);

	// If send failed, report the error, close the socket and remove the client
    if (SOCKET_ERROR == bytesSent) {
        reportError("Error at send()", false);
		client.setTerminated();
        closesocket(client.socket);
        clients.erase(client.socket);
        return;
    }
    std::cout << "Web Server: Sent: " << bytesSent << "/" << client.out_buffer.size() << " bytes of response.\n";

	// If not all bytes were sent, keep the remaining in out
    if (bytesSent < (int)client.out_buffer.size()) {
        client.out_buffer = client.out_buffer.substr(bytesSent);
		client.setTransmitting(); // Stay in Transmitting state
    } 
	// If all bytes were sent, clear out_buffer and reset state to Finished
    else {
		client.setFinished(); // Clear out_buffer and reset state to Finished
    }
}

void Server::run() {
	// If initialization failed, exit
    if (listenSocket == INVALID_SOCKET) {
        reportError("Initialization failed", false);
        return;
    }
	// Start listening for incoming connections
    if (!listen()) {
        reportError("Listen failed", true);
        return;
    }

    std::cout << "Server is running, waiting for connections...\n";
	// Main server loop
    while (true) {
		fd_set readfds, writefds;
		prepareFdSets(readfds, writefds); // Organize sockets into 'waiting to be read' and 'waiting to be written to' sets
        int nfd = select(0, &readfds, &writefds, NULL, NULL);

		// If select failed, report the error and exit
        if (nfd == SOCKET_ERROR) {
            reportError("Error at select()", false);
            return;
        }
		// If new connection is pending, accept it
        if (FD_ISSET(listenSocket, &readfds)) {
            acceptConnection();
        }
		// Handle all client events (read/write/cleanup)
        processClients(readfds, writefds);
    }
}

// Prepare socket sets for select()
void Server::prepareFdSets(fd_set& readfds, fd_set& writefds) {
	FD_ZERO(&readfds); // Initialize read set
	FD_ZERO(&writefds); // Initialize write set
	FD_SET(listenSocket, &readfds); // Add listening socket to read set to monitor new connections

    for (auto& kv : clients) {
        // --- INPUT SET (readfds) ---
        // Sockets added here are monitored for incoming data.
        // Client states that qualify:
        //   - AwaitingData: waiting for a new request or more data from client
        if (kv.second.state == ClientState::AwaitingData) {
            FD_SET(kv.first, &readfds);
        }

        // --- OUTPUT SET (writefds) ---
        // Sockets added here are monitored for readiness to send data.
        // Client states that qualify:
        //   - Transmitting: server has a response ready to send to client
        if (kv.second.state == ClientState::Transmitting) {
            FD_SET(kv.first, &writefds);
        }
    }
}

// Handle all client events (read/write/cleanup)
void Server::processClients(fd_set& readfds, fd_set& writefds) {

	// List of sockets to remove after iteration to avoid modifying map during iteration
    std::vector<SOCKET> removeList;

    for (auto& kv : clients) {
        SOCKET sock = kv.first;
        Client& client = kv.second;
		// If socket is ready for reading, receive message
        if (FD_ISSET(sock, &readfds)) {
            receiveMessage(client);
        }
		// If socket is ready for writing, send message
        if (FD_ISSET(sock, &writefds)) {
            sendMessage(client);
        }
		// If client is idle for too long, mark for termination
        if (is_client_idle(client)) {
            client.setTerminated();
        }
		// If client is in Terminated or Finished state, close socket and mark for removal
        if (client.state == ClientState::Terminated || client.state == ClientState::Finished) {
            closesocket(sock);
            removeList.push_back(sock);
        }
    }

	// Remove terminated clients from the map
    for (SOCKET sock : removeList) {
        clients.erase(sock);
    }
}