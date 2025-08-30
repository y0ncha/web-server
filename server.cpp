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
	// Close all client sockets
    for (auto& kv : clients) {
        closesocket(kv.second.socket);
    }
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

void Server::acceptConnection() {
    sockaddr_in from;
    int fromLen = sizeof(from);
    SOCKET msgSocket = accept(listenSocket, (sockaddr*)&from, &fromLen);
	// If accept failed, log error and return
    if (INVALID_SOCKET == msgSocket) {
		reportError("Error at accept()", 0);
        return;
    }
    std::cout << "Web Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << std::endl;
    unsigned long flag = 1;
	// Set the client socket to be non-blocking
    ioctlsocket(msgSocket, FIONBIO, &flag);
	// Add the new client to the clients map
    clients.emplace(std::piecewise_construct,
                   std::forward_as_tuple(msgSocket),
                   std::forward_as_tuple(msgSocket));
}

void Server::receiveMessage(SOCKET socket) {
    std::string tempBuff(recv_buffer_size_, '\0');
    int bytesRecv = recv(socket, &tempBuff[0], static_cast<int>(tempBuff.size() - 1), 0);
    if (SOCKET_ERROR == bytesRecv) {
        std::cout << "Web Server: Error at recv(): " << WSAGetLastError() << std::endl;
        closesocket(socket);
        clients.erase(socket);
        return;
    }
    if (bytesRecv == 0) {
        closesocket(socket);
        clients.erase(socket);
        return;
    }
    tempBuff.resize(bytesRecv); // keep only received bytes
    Client& client = clients[socket];
    client.last_active = time(nullptr);
    client.in_buffer.append(tempBuff);
    std::cout << "Web Server: Received: " << bytesRecv << " bytes of \"" << tempBuff << "\" message.\n";
    if (is_http_request_complete(client.in_buffer) && !client.parsed_request) {
        client.request_complete = true;
        client.parsed_request = std::make_unique<Request>(client.in_buffer);
        client.pending_response = std::make_unique<Response>();
        client.pending_response->body = "Hello, World!";
        client.pending_response->headers["Content-Type"] = "text/plain";
        client.pending_response->headers["Content-Length"] = std::to_string(client.pending_response->body.size());
        client.out_buffer = client.pending_response->to_string();
        client.ready_to_send = true;
    }
}

void Server::sendMessage(SOCKET socket) {
    Client& client = clients[socket];
    if (client.out_buffer.empty()) {
        client.ready_to_send = false;
        client.request_complete = false;
        client.parsed_request.reset();
        client.pending_response.reset();
        return;
    }
    int bytesSent = send(socket, client.out_buffer.c_str(), (int)client.out_buffer.size(), 0);
    if (SOCKET_ERROR == bytesSent) {
        std::cout << "Web Server: Error at send(): " << WSAGetLastError() << std::endl;
        closesocket(socket);
        clients.erase(socket);
        return;
    }
    std::cout << "Web Server: Sent: " << bytesSent << "/" << client.out_buffer.size() << " bytes of response.\n";
    if (bytesSent < (int)client.out_buffer.size()) {
        client.out_buffer = client.out_buffer.substr(bytesSent);
        client.ready_to_send = true;
    } else {
        client.out_buffer.clear();
        client.ready_to_send = false;
        client.request_complete = false;
        client.parsed_request.reset();
        client.pending_response.reset();
    }
}

void Server::run() {
    if (listenSocket == INVALID_SOCKET) {
        std::cout << "Web Server: Initialization failed.\n";
        return;
    }
    if (!listen()) {
        std::cout << "Web Server: Listen failed.\n";
        return;
    }
    std::cout << "Server is running, waiting for connections...\n";
    while (true) {
        fd_set readfds, writefds;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_SET(listenSocket, &readfds);
        for (auto& kv : clients) {
            FD_SET(kv.first, &readfds);
            if (kv.second.ready_to_send) {
                FD_SET(kv.first, &writefds);
            }
        }
        int nfd = select(0, &readfds, &writefds, NULL, NULL);
        if (nfd == SOCKET_ERROR) {
            std::cout << "Web Server: Error at select(): " << WSAGetLastError() << std::endl;
            return;
        }
        if (FD_ISSET(listenSocket, &readfds)) {
            acceptConnection();
        }
        std::vector<SOCKET> to_remove;
        for (auto& kv : clients) {
            SOCKET sock = kv.first;
            Client& client = kv.second;
            if (FD_ISSET(sock, &readfds)) {
                receiveMessage(sock);
            }
            if (FD_ISSET(sock, &writefds) && client.ready_to_send) {
                sendMessage(sock);
            }
            // Idle timeout
            if (is_client_idle(client)) {
                closesocket(sock);
                to_remove.push_back(sock);
            }
        }
        for (SOCKET sock : to_remove) {
            clients.erase(sock);
        }
    }
}
