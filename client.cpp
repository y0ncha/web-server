#include "client.h"

Client::Client(SOCKET s, const sockaddr_in& addr)
    : socket(s), last_active(0), state(ClientState::AwaitingRequest) {
    std::ostringstream oss;
    oss << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port);
    client_addr = oss.str();
}

Client::Client()
    : socket(INVALID_SOCKET), last_active(0), state(ClientState::Disconnected) {
    client_addr = "";
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
void Client::setFinished() {
    out_buffer.clear();
    parsed_request.reset();
    pending_response.reset();
    state = ClientState::Completed;
    std::cout << "Client [" << client_addr << "] state changed to Finished\n";
}
void Client::setTerminated() {
    state = ClientState::Abort;
    std::cout << "Client [" << client_addr << "] state changed to Terminated\n";
}

bool Client::isIdle(int timeout_sec) const {
    return (time(nullptr) - last_active > timeout_sec && state == ClientState::AwaitingRequest);
}
