#pragma once
#include <string>
#include <algorithm>
#include "client.h"
#include <ctime>

inline bool is_http_request_complete(const std::string& buffer) {
    size_t header_end = buffer.find("\r\n\r\n");
    if (header_end == std::string::npos) return false;
    size_t content_length = 0;
    size_t cl_pos = buffer.find("Content-Length:");
    if (cl_pos != std::string::npos && cl_pos < header_end) {
        size_t cl_end = buffer.find("\r\n", cl_pos);
        if (cl_end != std::string::npos) {
            std::string cl_val = buffer.substr(cl_pos + 15, cl_end - (cl_pos + 15));
            cl_val.erase(std::remove_if(cl_val.begin(), cl_val.end(), ::isspace), cl_val.end());
            try {
                content_length = std::stoul(cl_val);
            } catch (...) {
                content_length = 0;
            }
        }
    }
    size_t body_start = header_end + 4;
    size_t body_len = buffer.size() - body_start;
    if ((content_length == 0 && body_len == 0) || (content_length > 0 && body_len >= content_length)) {
        return true;
    }
    return false;
}

inline bool is_client_idle(const Client& client, int timeout_sec = 120) {
    return time(nullptr) - client.last_active > timeout_sec;
}
