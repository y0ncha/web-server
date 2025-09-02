#pragma once
#include "response.h"
#include "request.h"
#include <string>

Response handle_health();
Response handle_echo(const Request& req);
Response handle_root(const Request& req);
Response handle_not_found();

// Returns Content-Length value from HTTP headers, or 0 if not found
size_t get_content_length(const std::string& raw_headers);

// Returns true if the HTTP request is complete (headers and body)
bool is_request_complete(const std::string& buffer);

// Returns true if the connection should be kept alive based on HTTP version and Connection header
bool is_keep_alive(const Request& req);