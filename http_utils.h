#pragma once
#include <string>
#include <algorithm>
#include "client.h"
#include <ctime>

inline bool is_client_idle(const Client& client, int timeout_sec = 120) {
    return time(nullptr) - client.last_active > timeout_sec;
}
