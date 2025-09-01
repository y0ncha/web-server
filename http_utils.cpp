#include "http_utils.h"

// Returns Content-Length value from HTTP headers, or 0 if not found
size_t get_content_length(const std::string& raw_headers) {
    size_t cl_pos = raw_headers.find("Content-Length:");
    if (cl_pos == std::string::npos) return 0;
    size_t cl_end = raw_headers.find("\r\n", cl_pos);
    if (cl_end == std::string::npos) return 0;
    std::string cl_val = raw_headers.substr(cl_pos + 15, cl_end - (cl_pos + 15));
    cl_val.erase(std::remove_if(cl_val.begin(), cl_val.end(), ::isspace), cl_val.end());
    try {
        return std::stoul(cl_val);
    }
    catch (...) {
        return 0;
    }
}

bool is_request_complete(const std::string& buffer) {
    size_t header_end = buffer.find("\r\n\r\n");
    if (header_end == std::string::npos) return false;
    size_t content_length = get_content_length(buffer.substr(0, header_end));
    size_t body_start = header_end + 4;
    size_t body_len = buffer.size() - body_start;
    if ((content_length == 0 && body_len == 0) || (content_length > 0 && body_len >= content_length)) {
        return true;
    }
    return false;
}

// A simple health check endpoint response
Response health() {
    return Response::ok();
}
