#include "server.h"
#include <iostream>
#include <csignal>

static constexpr const char* IP = "127.0.0.1";
static constexpr int PORT = 27015;

int main() {
    Server server(IP, PORT);
	server.run();
    return 0;
}
