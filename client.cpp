#include "client.h"

Client::Client(SOCKET s)
    : socket(s), ready_to_send(false), request_complete(false) {
    last_active = time(nullptr);
}
// No need for manual delete, unique_ptr handles cleanup