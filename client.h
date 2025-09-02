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

static constexpr size_t BUFF_SIZE = 1024; // 4KB max buffer size

// Client states for the FSM
enum class ClientState {
    Disconnected,   // No active client connection
    AwaitingRequest,   // Waiting for a new request
    RequestBuffered,   // Full request buffered
    ResponseReady,     // Response is ready
    Completed,          // Done, ready for next or close
    Aborted         // Socket should be closed
};

class Client {
public:

    SOCKET socket;                  // Client socket descriptor
    std::string client_addr;        // Store client address
    std::string in_buffer;          // Raw incoming data buffer
    std::string out_buffer;         // Fully constructed HTTP response
    time_t last_active;             // Used for idle timeout tracking
	bool keep_alive;                // Connection: keep-alive or close

    ClientState state;

    Client(SOCKET s, const sockaddr_in& addr); // Add constructor for socket and address
    Client();
    ~Client();
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    void setDisconnected();
    void setAwaitingRequest();
    void setRequestBuffered();
    void setResponseReady();
    void setCompleted();
    void setAborted();

    bool isIdle(int timeout_sec = 120) const;
    void bufferRequest(const std::string& data);
};