#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include <string>
#include <iostream>
#include <map>
#include <ctime>
#include <algorithm>
#include <vector>
#include <fstream>
#include <sstream>
#include "client.h"
#include "utils.h"
#include "http_utils.h"

/**
 * Main Server class for TCP non-blocking async HTTP server.
 * Handles event loop, client management, and request dispatching.
 */
class Server {
public:
	// Constructor: initializes Winsock and sets up the listening socket.
    Server(const std::string& ip, int port, std::size_t bufferSize = 1024, std::time_t idleTImeout = 120);
	// Destructor: cleans up all client connections and Winsock.
    ~Server();
	// Main server loop: handles connections and client events.
    void run();
private:
    std::string ip_; // Server IP address
    int port_;       // Server port
    SOCKET listenSocket; // Listening socket
    std::map<SOCKET, Client> clients; // Connected clients
    const std::size_t BUFF_SIZE; // Max size of the buffer
    const time_t CLIENT_TIMEOUT; // 2 minutes
    long long iteration; // Loop iteration counter

    // Starts listening for incoming connections
    bool listen();
    // Accepts a new client connection
    void acceptConnection();
    // Receives a message from a client
    void receiveMessage(Client& client);
    // Sends a message to a client
    void sendMessage(Client& client);
    // Adds a new client to the clients map
    bool addClient(SOCKET clientSocket, const sockaddr_in& addr);
    // Prepares socket sets for select()
    void prepareFdSets(fd_set& readfds, fd_set& writefds, fd_set& errorfds);
    // Polls sockets for events using select()
    bool pollEvents(fd_set& readfds, fd_set& writefds, fd_set& errorfds);
    // Processes a client based on its state and socket readiness
    void processClient(Client& client, fd_set& readfds, fd_set& writefds, fd_set& errorfds);
    // Dispatches the request to the appropriate handler and prepares the response
    void dispatch(Client& client); // FSM: RequestBuffered → ResponseReady
};

