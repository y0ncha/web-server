#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <string>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

class Server {
public:
    Server();
    ~Server();
    void run();
private:
    enum class SocketType {
        Empty = 0,
        Listen = 1,
        Receive = 2,
        Idle = 3,
        Send = 4
    };
    enum class SendSubType {
        None = 0,
        Time = 1,
        Seconds = 2
    };
    struct SocketState {
        SOCKET id = INVALID_SOCKET;
        SocketType recv = SocketType::Empty;
        SocketType send = SocketType::Idle;
        SendSubType sendSubType = SendSubType::None;
        std::string buffer;
    };
    static const int SERVER_PORT = 27015;
    static const int MAX_SOCKETS = 60;
    SocketState sockets[MAX_SOCKETS] = {};
    int socketsCount = 0;
    SOCKET listenSocket = INVALID_SOCKET;
    bool addSocket(SOCKET id, SocketType what);
    void removeSocket(int index);
    void acceptConnection(int index);
    void receiveMessage(int index);
    void sendMessage(int index);
};

