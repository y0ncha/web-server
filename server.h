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
#include "client.h"
#include "utils.h"
#include "http_utils.h"

class Server {
public:
    Server(const std::string& ip, int port, std::size_t buffer_size = 1024);
    ~Server();
    void run();
private:
    std::string ip_;
    int port_;

    SOCKET listenSocket;
    std::map<SOCKET, Client> clients;
	const std::size_t BUFF_SIZE; // Max size of the buffer
	const time_t CLIENT_TIMEOUT = 120; // 2 minutes

    bool listen();
    void acceptConnection();
    void receiveMessage(Client& client);
    void sendMessage(Client& client);
    void reportError(const std::string& msg, bool cleanupWSA);
    bool addClient(SOCKET clientSocket, const sockaddr_in& addr);
    void prepareFdSets(fd_set& readfds, fd_set& writefds);
	bool pollEvents(fd_set& readfds, fd_set& writefds);
    void processClient(Client& client, fd_set& readfds, fd_set& writefds);
    void dispatch(Client& client); // FSM: RequestBuffered ? ResponseReady
};

