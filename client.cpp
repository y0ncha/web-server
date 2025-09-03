#include "client.h"

/**
 * @brief Constructs a client with socket and address.
 * @param s Socket descriptor
 * @param addr Client address
 */
Client::Client(SOCKET s, const sockaddr_in& addr)
    : socket(s), lastActive(0), keepAlive(true), state(ClientState::AwaitingRequest) {
    std::ostringstream oss;
    oss << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port);
    clientAddr = oss.str();
    inBuffer.reserve(BUFF_SIZE);
    outBuffer.reserve(BUFF_SIZE);
}

/**
 * @brief Default constructor for Client.
 */
Client::Client()
    : socket(INVALID_SOCKET), lastActive(0), keepAlive(true), state(ClientState::Disconnected) {
    clientAddr = "";
    inBuffer.reserve(BUFF_SIZE);
    outBuffer.reserve(BUFF_SIZE);
}

/**
 * @brief Destructor for Client.
 */
Client::~Client() {}

/**
 * @brief Sets client state to Disconnected.
 */
void Client::setDisconnected() {
    state = ClientState::Disconnected;
    std::cout << "Client [" << clientAddr << "] state changed to Disconnected\n";
}

/**
 * @brief Sets client state to AwaitingRequest.
 */
void Client::setAwaitingRequest() {
    lastActive = time(nullptr);
    state = ClientState::AwaitingRequest;
    std::cout << "Client [" << clientAddr << "] state changed to AwaitingRequest\n";
}

/**
 * @brief Sets client state to RequestBuffered.
 */
void Client::setRequestBuffered() {
    lastActive = time(nullptr);
    state = ClientState::RequestBuffered;
    std::cout << "Client [" << clientAddr << "] state changed to RequestBuffered\n";
}

/**
 * @brief Sets client state to ResponseReady.
 */
void Client::setResponseReady() {
    state = ClientState::ResponseReady;
    std::cout << "Client [" << clientAddr << "] state changed to ResponseReady\n";
}

/**
 * @brief Sets client state to Completed.
 */
void Client::setCompleted() {
    inBuffer.clear();
    outBuffer.clear();
    state = ClientState::Completed;
    std::cout << "Client [" << clientAddr << "] state changed to Completed\n";
}

/**
 * @brief Sets client state to Aborted.
 */
void Client::setAborted() {
    state = ClientState::Aborted;
    std::cout << "Client [" << clientAddr << "] state changed to Aborted\n";
}

/**
 * @brief Checks if client is idle for longer than timeoutSec seconds.
 * @param timeoutSec Timeout in seconds
 * @return True if idle, false otherwise
 */
bool Client::isIdle(int timeoutSec) const {
    return (difftime(time(nullptr), lastActive) > timeoutSec && state == ClientState::AwaitingRequest);
}

/**
 * @brief Buffers incoming request data.
 * @param data Incoming data
 */
void Client::bufferRequest(const std::string& data) {
    inBuffer.append(data);
    setRequestBuffered();
}