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
    Server(const std::string& ip, int port, std::size_t buffer_size = 512);
    ~Server();
    void run();
private:
    std::string ip_;
    int port_;
    SOCKET listenSocket = INVALID_SOCKET;
    sockaddr_in serverService_;
    std::map<SOCKET, Client> clients;
    const std::size_t recv_buffer_size_;
    bool listen();
    void acceptConnection();
    void receiveMessage(SOCKET socket);
    void sendMessage(SOCKET socket);
    void reportError(const std::string& msg, bool cleanupWSA);
};

