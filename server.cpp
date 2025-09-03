#include "server.h"

/**
 * @brief Server constructor: initializes Winsock and sets up the listening socket.
 * @param ip Server IP address
 * @param port Server port
 * @param bufferSize Buffer size for client data
 */
Server::Server(const std::string& ip, int port, std::size_t bufferSize)
    : ip_(ip), port_(port), BUFF_SIZE(bufferSize)
{
    WSAData wsaData;
    if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        reportError("Error at WSAStartup()", 0);
        listenSocket = INVALID_SOCKET;
        return;
    }
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == listenSocket) {
        reportError("Error at socket()", 1);
        return;
    }
    sockaddr_in serverService;
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = inet_addr(ip_.c_str());
    serverService.sin_port = htons(port_);
    if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService))) {
        reportError("Error at bind()", 1);
        return;
    }
}

/**
 * @brief Server destructor: cleans up all client connections and Winsock.
 */
Server::~Server() {
    for (auto& kv : clients) {
        closesocket(kv.first);
    }
    clients.clear();
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
    }
    WSACleanup();
    std::cout << "Web Server: Closing Connection.\n";
}

/**
 * @brief Reports an error and optionally cleans up Winsock.
 * @param message Error message
 * @param cleanupWSA Whether to cleanup Winsock
 */
void Server::reportError(const std::string& message, bool cleanupWSA) {
    std::cerr << "Web Server: " << message << " Error: " << WSAGetLastError() << std::endl;
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
    if (cleanupWSA) {
        WSACleanup();
    }
}

/**
 * @brief Starts listening for incoming connection requests.
 * @return True if successful, false otherwise
 */
bool Server::listen() {
    if (SOCKET_ERROR == ::listen(listenSocket, 5)) {
        reportError("Error at listen()", true);
        return false;
    }
    unsigned long flag = 1;
    if (ioctlsocket(listenSocket, FIONBIO, &flag) != NO_ERROR) {
        reportError("Error at ioctlsocket()", true);
        return false;
    }
    return true;
}

/**
 * @brief Adds a new client to the clients map.
 * @param clientSocket Client socket
 * @param addr Client address
 * @return True if successful, false otherwise
 */
bool Server::addClient(SOCKET clientSocket, const sockaddr_in& addr) {
    unsigned long flag = 1;
    if (ioctlsocket(clientSocket, FIONBIO, &flag) != NO_ERROR) {
        closesocket(clientSocket);
        return false;
    }
    auto result = clients.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(clientSocket),
        std::forward_as_tuple(clientSocket, addr)
    );
    if (!result.second) {
        closesocket(clientSocket);
        return false;
    }
    std::cout << "Client [" << result.first->second.clientAddr << "] connected.\n";
    return true;
}

/**
 * @brief Dispatches the request to the appropriate handler and prepares the response.
 * @param client Reference to client object
 */
void Server::dispatch(Client& client) {
    Request request(client.inBuffer); // Parse the buffered request
    client.inBuffer.clear();
    client.keepAlive = isKeepAlive(request);
    Response response;
    if (request.method == "GET") {
        if (request.path == "/health") {
            response = handleHealth();
        }
        else if (request.path == "/echo") {
            response = handleEcho(request);
        }
        else {
            response = handleHtmlFile(request);
        }
    }
    else {
        response = handleBadRequest("Unsupported HTTP method");
    }
    client.outBuffer = response.toString();
    client.setResponseReady();
}

/**
 * @brief Accepts a new client connection.
 */
void Server::acceptConnection() {
    sockaddr_in from;
    int fromLen = sizeof(from);
    SOCKET clientSocket = accept(listenSocket, (sockaddr*)&from, &fromLen);
    if (INVALID_SOCKET == clientSocket) {
        reportError("Error at accept()", 0);
        return;
    }
    if (addClient(clientSocket, from)) {
        clients[clientSocket].setAwaitingRequest();
    }
}

/**
 * @brief Receives a message from the client and buffers it.
 * @param client Reference to client object
 */
void Server::receiveMessage(Client& client) {
    if (client.state != ClientState::AwaitingRequest) {
        reportError("receiveMessage called in invalid client state", false);
    }
    std::string recvBuffer(BUFF_SIZE, '\0');
    int bytesRecv = recv(client.socket, &recvBuffer[0], static_cast<int>(recvBuffer.size() - 1), 0);
    if (SOCKET_ERROR == bytesRecv) {
        client.setAborted();
        reportError("Error at recv()", false);
        closesocket(client.socket);
        return;
    }
    if (bytesRecv == 0) {
        client.setCompleted();
        closesocket(client.socket);
        return;
    }
    recvBuffer.resize(bytesRecv);
    client.inBuffer.append(recvBuffer);
    // If incomplete request, keep buffering
    if (!isRequestComplete(recvBuffer)) {
        std::cout << "Web Server: Received " << bytesRecv << " bytes (partial). Total buffered: "
            << client.inBuffer.size() << " bytes.\n";
        return;
    }
    client.setRequestBuffered();
    std::cout << "Web Server: Received " << bytesRecv << " bytes\n";
}

/**
 * @brief Sends a message to the client.
 * @param client Reference to client object
 */
void Server::sendMessage(Client& client) {
    if (client.state != ClientState::ResponseReady || client.outBuffer.empty()) {
        reportError("sendMessage called in invalid state or empty buffer", false);
        return;
    }
    int bytesSent = send(client.socket, client.outBuffer.c_str(), (int)client.outBuffer.size(), 0);
    if (SOCKET_ERROR == bytesSent) {
        client.setAborted();
        reportError("Error at send()", false);
        closesocket(client.socket);
        return;
    }
    if (bytesSent < client.outBuffer.size()) {
        client.outBuffer = client.outBuffer.substr(bytesSent);
        std::cout << "Web Server: Sent " << bytesSent << " bytes (partial). Remaining: "
            << client.outBuffer.size() - bytesSent << " bytes.\n";
        return;
    }
    std::cout << "Web Server: Sent " << bytesSent << "/" << client.outBuffer.size() << " bytes of response.\n";
    client.outBuffer.clear();
    client.keepAlive ? client.setAwaitingRequest() : client.setCompleted();
}

/**
 * @brief Main server loop: handles connections and client events.
 */
void Server::run() {
    if (listenSocket == INVALID_SOCKET) {
        reportError("Initialization failed", false);
        return;
    }
    if (!listen()) {
        reportError("Listen failed", true);
        return;
    }
    std::cout << "Server is running, waiting for connections...\n";
    while (true) {
        fd_set readfds, writefds;
        if (!pollEvents(readfds, writefds)) {
            reportError("Polling events failed", true);
            return;
        }
        if (FD_ISSET(listenSocket, &readfds)) {
            acceptConnection();
        }
        std::vector<SOCKET> clientsToRemove;
        for (auto& kv : clients) {
            Client& client = kv.second;
            processClient(client, readfds, writefds);
            if (client.state == ClientState::Aborted || client.state == ClientState::Completed) {
                closesocket(client.socket);
                clientsToRemove.push_back(client.socket);
            }
        }
        for (SOCKET sock : clientsToRemove) {
            clients.erase(sock);
        }
    }
}

/**
 * @brief Prepares socket sets for select().
 * @param readfds Read file descriptor set
 * @param writefds Write file descriptor set
 */
void Server::prepareFdSets(fd_set& readfds, fd_set& writefds) {
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_SET(listenSocket, &readfds);
    for (auto& kv : clients) {
        if (kv.second.state == ClientState::AwaitingRequest) {
            FD_SET(kv.first, &readfds);
        }
        if (kv.second.state == ClientState::ResponseReady) {
            FD_SET(kv.first, &writefds);
        }
    }
}

/**
 * @brief Polls sockets for events using select().
 * @param readfds Read file descriptor set
 * @param writefds Write file descriptor set
 * @return True if successful, false otherwise
 */
bool Server::pollEvents(fd_set& readfds, fd_set& writefds) {
    prepareFdSets(readfds, writefds);
    int nfd = select(0, &readfds, &writefds, NULL, NULL);
    if (nfd == SOCKET_ERROR) {
        reportError("Error at select()", false);
        return false;
    }
    return true;
}

/**
 * @brief Processes a client based on its state and socket readiness.
 * @param client Reference to client object
 * @param readfds Read file descriptor set
 * @param writefds Write file descriptor set
 */
void Server::processClient(Client& client, fd_set& readfds, fd_set& writefds) {
    SOCKET sock = client.socket;
    if (FD_ISSET(sock, &readfds) && client.state == ClientState::AwaitingRequest) {
        receiveMessage(client);
    }
    if (client.state == ClientState::RequestBuffered) {
        dispatch(client);
    }
    if (FD_ISSET(sock, &writefds) && client.state == ClientState::ResponseReady) {
        sendMessage(client);
    }
    else if (client.isIdle()) {
        client.setAborted();
    }
}