#include "client.h"

Client::Client(SOCKET s, const sockaddr_in& addr)
    : socket(s), last_active(0), state(ClientState::AwaitingRequest) {
    std::ostringstream oss;
    oss << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port);
    client_addr = oss.str();
    in_buffer.reserve(512);
    out_buffer.reserve(512);
}

Client::Client()
    : socket(INVALID_SOCKET), last_active(0), state(ClientState::Disconnected) {
    client_addr = "";
    in_buffer.reserve(512);
	out_buffer.reserve(512);
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
void Client::setRequestBuffered(const std::string& data) {
    last_active = time(nullptr);
    in_buffer.append(data);
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
void Client::setAbort() {
    state = ClientState::Abort;
    std::cout << "Client [" << client_addr << "] state changed to Aborted\n";
}

bool Client::isIdle(int timeout_sec) const {
    return (time(nullptr) - last_active > timeout_sec && state == ClientState::AwaitingRequest);
}
