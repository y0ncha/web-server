#include "client.h"
#include <ctime>

Client::Client(SOCKET s, const sockaddr_in& addr)
    : socket(s), last_active(0),keep_alive(true), state(ClientState::AwaitingRequest) {
    std::ostringstream oss;
    oss << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port);
    client_addr = oss.str();
    in_buffer.reserve(BUFF_SIZE);
    out_buffer.reserve(BUFF_SIZE);
}

Client::Client()
    : socket(INVALID_SOCKET), last_active(0),keep_alive(true), state(ClientState::Disconnected) {
    client_addr = "";
    in_buffer.reserve(BUFF_SIZE);
	out_buffer.reserve(BUFF_SIZE);
}

Client::~Client() {}

void Client::setDisconnected() {
    state = ClientState::Disconnected;
    std::cout << "Client [" << client_addr << "] state changed to Disconnected\n";
}
void Client::setAwaitingRequest() {
	last_active = time(nullptr); // For idle timeout tracking
    state = ClientState::AwaitingRequest;
    std::cout << "Client [" << client_addr << "] state changed to AwaitingRequest\n";
}
void Client::setRequestBuffered() {
    last_active = time(nullptr);
    state = ClientState::RequestBuffered;
    std::cout << "Client [" << client_addr << "] state changed to RequestBuffered\n";
}
void Client::setResponseReady() {
    state = ClientState::ResponseReady;
    std::cout << "Client [" << client_addr << "] state changed to ResponseReady\n";
}
void Client::setCompleted() {
	in_buffer.clear();
    out_buffer.clear();
    state = ClientState::Completed;
    std::cout << "Client [" << client_addr << "] state changed to Completed\n";
}
void Client::setAborted() {
    state = ClientState::Aborted;
    std::cout << "Client [" << client_addr << "] state changed to Aborted\n";
}

bool Client::isIdle(int timeout_sec) const {
    return (difftime(time(nullptr), last_active) > timeout_sec && state == ClientState::AwaitingRequest);
}

void Client::bufferRequest(const std::string& data) {
	in_buffer.append(data);
    setRequestBuffered();
}