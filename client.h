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

class Client {
public:
    SOCKET socket;                  // Client socket descriptor
    std::string in_buffer;          // Raw incoming data buffer
    std::string out_buffer;         // Fully constructed HTTP response
    bool ready_to_send = false;     // Flag: is response ready to send
    bool request_complete = false;  // Flag: did we receive full request
    time_t last_active = 0;         // Used for idle timeout

    std::unique_ptr<Request> parsed_request = nullptr;   // Parsed HTTP request (optional caching)
    std::unique_ptr<Response> pending_response = nullptr; // Pending HTTP response (if used)

    explicit Client(SOCKET s);
    Client() : socket(INVALID_SOCKET) {}
    ~Client() = default;
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
};