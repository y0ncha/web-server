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
 * @brief Enum representing the state of a client connection.
 * @details Used for managing the client's lifecycle in the server.
 */
enum class ClientState {
    Disconnected,      // No active client connection
    AwaitingRequest,   // Waiting for a new request
    RequestBuffered,   // Full request buffered
    ResponseReady,     // Response is ready
    Completed,         // Done, ready for next or close
    Aborted            // Socket should be closed
};

/**
 * @brief Represents a connected client and its state.
 * @details Manages the client's socket, buffers, state, and timing.
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

	// Default empty constructor for Client.
    Client();

	// Destructor for Client.
    ~Client();

	// Delete copy constructor and assignment operator to prevent copying
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

	// State transition methods
    void setDisconnected();
    void setAwaitingRequest();
    void setRequestBuffered();
    void setResponseReady();
    void setCompleted();
    void setAborted();
    
	// Checks if the client has been idle for longer than timeoutSec seconds.
    bool isIdle(int timeoutSec = 120) const;

	// Buffers incoming data into inBuffer
    void bufferRequest(const std::string& data);
};