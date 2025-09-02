#include "server.h"
#include "http_utils.h"
#include <algorithm> // For std::transform
#include <fstream> // For std::ifstream
#include <sstream> // For std::ostringstream

Server::Server(const std::string& ip, int port, std::size_t buffer_size)
    : ip_(ip), port_(port), BUFF_SIZE(buffer_size)
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
	// Clean up all client connections (if any)
    for (auto& kv : clients) {
        closesocket(kv.first); // Ensure socket is closed
    }
    clients.clear(); // Remove all clients

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
	std::cout << "Client [" << result.first->second.client_addr << "] connected.\n";
    return true;
}

void Server::dispatch(Client& client) {

	Request req(client.in_buffer); // Parse the buffered request
    client.in_buffer.clear(); // Clear input buffer after parsing
    client.keep_alive = is_keep_alive(req); // Determine if connection should be kept alive (defaulted to keep-alive)

    Response res;

    if (req.method == "GET") {
        if (req.path == "/health") {
            res = handle_health();
        } 
        else if (req.path == "/echo") {
			res = handle_echo(req);
        }
        else if (req.path == "/") {
            res = handle_root(req);
        }
    } 
    else {
        res = handle_not_found();
    }
    client.out_buffer = res.toString();
	client.setResponseReady(); // FSM: RequestBuffered ? ResponseReady
}

void Server::acceptConnection() {

	// Accept a new client connection
    sockaddr_in from;
    int fromLen = sizeof(from);
    SOCKET msgSocket = accept(listenSocket, (sockaddr*)&from, &fromLen);

	// If accept failed, report error and return
    if (INVALID_SOCKET == msgSocket) {
        reportError("Error at accept()", 0);
        return;
    }
	// Add the new client to the clients map
    if (addClient(msgSocket, from)) {
        clients[msgSocket].setAwaitingRequest(); // FSM: Disconnected ? AwaitingRequest
    }
}

void Server::receiveMessage(Client& client) {

	// If not in correct state, report error, abort client and return
    if (client.state != ClientState::AwaitingRequest) {
		reportError("receiveMessage called in invalid client state", false); // For now just report error
    }

	// Receive data from the client
    std::string tempBuff(BUFF_SIZE, '\0');
    int bytesRecv = recv(client.socket, &tempBuff[0], static_cast<int>(tempBuff.size() - 1), 0);

	// If recv failed, report error, abort client and remove from map
    if (SOCKET_ERROR == bytesRecv) {
        reportError("Error at recv()", false);
        client.setAborted();
        closesocket(client.socket);
        clients.erase(client.socket);
        return;
    }
	// If connection was gracefully closed, mark client as complete and remove from map
    if (bytesRecv == 0) {
        client.setCompleted();
        closesocket(client.socket);
        clients.erase(client.socket);
        return;
    }

	// Successfully received data
	tempBuff.resize(bytesRecv); // Resize to actual received size
	client.in_buffer.append(tempBuff); // Append to client's input buffer

	// If incomplete request, keep buffering
    if (!is_request_complete(tempBuff)) {
        std::cout << "Web Server: Received " << bytesRecv << " bytes (partial). Total buffered: "
            << client.in_buffer.size() << " bytes.\n";
		return;
    }
	client.setRequestBuffered(); // FSM: AwaitingRequest ? RequestBuffered
    std::cout << "Web Server: Received " << bytesRecv << " bytes\n";
	// std::cout << client.in_buffer << "\n"; // debug
}

void Server::sendMessage(Client& client) {

	// If not in correct state or buffer is empty, report error and return
    if (client.state != ClientState::ResponseReady || client.out_buffer.empty()) {
		reportError("sendMessage called in invalid state or empty buffer", false);
        return;
    }

    int bytesSent = send(client.socket, client.out_buffer.c_str(), (int)client.out_buffer.size(), 0);
	// If send failed, report error, abort client and remove from map
    if (SOCKET_ERROR == bytesSent) {
        reportError("Error at send()", false);
        client.setAborted();
        closesocket(client.socket);
        clients.erase(client.socket);
        return;
    }
    if (bytesSent < client.out_buffer.size()) {
        client.out_buffer = client.out_buffer.substr(bytesSent); // Remove sent part
        std::cout << "Web Server: Sent " << bytesSent << " bytes (partial). Remaining: "
            << client.out_buffer.size() - bytesSent << " bytes.\n";
		return; // Wait for next opportunity to send remaining data
    }
    std::cout << "Web Server: Sent " << bytesSent << "/" << client.out_buffer.size() << " bytes of response.\n";
	client.out_buffer.clear(); // Clear output buffer after sending

	client.keep_alive ? client.setAwaitingRequest() : client.setCompleted(); // FSM: ResponseReady ? (AwaitingRequest | Completed)
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

		// Prepare socket sets and poll for events
		fd_set readfds, writefds;
		if (!pollEvents(readfds, writefds)) {
            reportError("Polling events failed", true);
            return;
		}

		// If new connection is pending, accept it
        if (FD_ISSET(listenSocket, &readfds)) {
            acceptConnection();
        }

        // Process all client events (read/write/cleanup)
        std::vector<SOCKET> clientsToRemove;

        for (auto& kv : clients) {
            
			// Process each client based on its state and socket readiness
            Client& client = kv.second;
            processClient(client, readfds, writefds);

			// If client is completed or aborted, close socket and mark for removal (to avoid iterator invalidation)
            if (client.state == ClientState::Aborted || client.state == ClientState::Completed) {
                closesocket(client.socket);
                clientsToRemove.push_back(client.socket);
            }
        }
        // Remove terminated clients from the map
        for (SOCKET sock : clientsToRemove) {
            clients.erase(sock);
        }
    }
}

// Prepare socket sets for select()
void Server::prepareFdSets(fd_set& readfds, fd_set& writefds) {

	// Clear the sets and add the listening socket
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_SET(listenSocket, &readfds);

	// Add client sockets based on their state
    for (auto& kv : clients) {
        if (kv.second.state == ClientState::AwaitingRequest) {
            FD_SET(kv.first, &readfds);
        }
        if (kv.second.state == ClientState::ResponseReady) {
            FD_SET(kv.first, &writefds);
        }
    }
}

// Poll sockets for events using select()
bool Server::pollEvents(fd_set& readfds, fd_set& writefds) {

	prepareFdSets(readfds, writefds);
    int nfd = select(0, &readfds, &writefds, NULL, NULL);

    if (nfd == SOCKET_ERROR) {
        reportError("Error at select()", false);
        return false;
    }
    return true;
}

void Server::processClient(Client& client, fd_set& readfds, fd_set& writefds) {
    SOCKET sock = client.socket;

    // If socket is ready to read and client is awaiting request, receive message
    if (FD_ISSET(sock, &readfds) && client.state == ClientState::AwaitingRequest) {
        receiveMessage(client);
    }

    // If client has a buffered request, dispatch to process it
    if (client.state == ClientState::RequestBuffered) {
        dispatch(client);
    }

    // If socket is ready to write and response is ready, send message
    if (FD_ISSET(sock, &writefds) && client.state == ClientState::ResponseReady) {
        sendMessage(client);
    }

    // If client is idle for too long, terminate it
    else if (client.isIdle()) {
        client.setAborted();
    }
}