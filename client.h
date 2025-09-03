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

/**
 * Client connection states for the FSM.
 */ 
enum class ClientState {
    Disconnected,   // No active client connection
    AwaitingRequest,   // Waiting for a new request
    RequestBuffered,   // Full request buffered
    ResponseReady,     // Response is ready
    Completed,         // Done, ready for next or close
    Aborted            // Socket should be closed
};


/**
 * Represents a connected client with its state and buffers.
 * Implements a simple FSM for connection handling.
 */
class Client {
public:
    SOCKET socket;                  // Client socket descriptor
    std::string clientAddr;         // Store client address
    std::string inBuffer;           // Raw incoming data buffer
    std::string outBuffer;          // Fully constructed HTTP response
    time_t lastActive;              // Used for idle timeout tracking
    bool keepAlive;                 // Connection: keep-alive or close
    ClientState state;

    // Constructs a client with socket and address.
    Client(SOCKET s, const sockaddr_in& addr);
    // Default constructor for Client.
    Client();
    ~Client();
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    // Sets client state to Disconnected.
    void setDisconnected();
    // Sets client state to AwaitingRequest.
    void setAwaitingRequest();
    // Sets client state to RequestBuffered.
    void setRequestBuffered();
    // Sets client state to ResponseReady.
    void setResponseReady();
    // Sets client state to Completed.
    void setCompleted();
    // Sets client state to Aborted.
    void setAborted();

    // Checks if client is idle for longer than timeoutSec seconds.
    bool isIdle(int timeoutSec = 120) const;
    // Buffers incoming request data.
    void bufferRequest(const std::string& data);
};