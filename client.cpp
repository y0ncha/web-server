#include "client.h"

// Helper to convert ClientState to string
static std::string clientStateToString(ClientState state) {
    switch (state) {
        case ClientState::Disconnected: return "Disconnected";
        case ClientState::AwaitingRequest: return "AwaitingRequest";
        case ClientState::RequestBuffered: return "RequestBuffered";
        case ClientState::ResponseReady: return "ResponseReady";
        case ClientState::Completed: return "Completed";
        case ClientState::Aborted: return "Aborted";
        default: return "Unknown";
    }
}

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
    std::string oldState = clientStateToString(state);
    state = ClientState::Disconnected;
    logClientState(clientAddr, oldState, clientStateToString(state));
}

/**
 * @brief Sets client state to AwaitingRequest.
 */
void Client::setAwaitingRequest() {
    std::string oldState = clientStateToString(state);
    lastActive = time(nullptr);
    state = ClientState::AwaitingRequest;
    logClientState(clientAddr, oldState, clientStateToString(state));
}

/**
 * @brief Sets client state to RequestBuffered.
 */
void Client::setRequestBuffered() {
    std::string oldState = clientStateToString(state);
    lastActive = time(nullptr);
    state = ClientState::RequestBuffered;
    logClientState(clientAddr, oldState, clientStateToString(state));
}

/**
 * @brief Sets client state to ResponseReady.
 */
void Client::setResponseReady() {
    std::string oldState = clientStateToString(state);
    state = ClientState::ResponseReady;
    logClientState(clientAddr, oldState, clientStateToString(state));
}

/**
 * @brief Sets client state to Completed.
 */
void Client::setCompleted() {
    std::string oldState = clientStateToString(state);
    inBuffer.clear();
    outBuffer.clear();
    state = ClientState::Completed;
    logClientState(clientAddr, oldState, clientStateToString(state));
}

/**
 * @brief Sets client state to Aborted.
 */
void Client::setAborted() {
    std::string oldState = clientStateToString(state);
    state = ClientState::Aborted;
    logClientState(clientAddr, oldState, clientStateToString(state));
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