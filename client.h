#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include <string>
#include <iostream>
#include <memory>
#include "request.h"
#include "response.h"
#include <sstream>

// Client state flow:
//accept()
//? AwaitingData
//? recv()
//? setRequestBuffered()
//? RequestBuffered
//? dispatch logic(manual or next step)
//? build response
//? Transmitting
//? send()
//? Finished
//? Terminated
//? closesocket() and remove from map
// 
// Error path:
// recv() / send() ? SOCKET_ERROR ? Terminated

enum class ClientState {
    Disconnected,   // No active client connection
    AwaitingRequest,   // Waiting for a new request
    RequestBuffered,   // Full request buffered
    ResponseReady,     // Response is ready
    Completed,          // Done, ready for next or close
    Abort         // Socket should be closed
};

class Client {
public:

    SOCKET socket;                  // Client socket descriptor
    std::string client_addr;        // Store client address
    std::string in_buffer;          // Raw incoming data buffer
    std::string out_buffer;         // Fully constructed HTTP response
    time_t last_active;             // Used for idle timeout tracking

    ClientState state;

    Client(SOCKET s, const sockaddr_in& addr); // Add constructor for socket and address
    Client();
    ~Client();
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    void setDisconnected();
    void setAwaitingRequest();
    void setRequestBuffered(const std::string& data);
    void setResponseReady();
    void setCompleted();
    void setAbort();
    bool isIdle(int timeout_sec = 120) const;
};