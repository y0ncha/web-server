#include "client.h"

Client::Client(SOCKET s) : socket(s),last_active(0), state(ClientState::AwaitingData) {}

void Client::bufferRequest(const std::string& data) {
    last_active = time(nullptr);
    in_buffer.append(data);
    state = ClientState::RequestBuffered;
}