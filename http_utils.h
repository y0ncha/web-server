#pragma once
#include <string>
#include <algorithm>
#include "client.h"
#include <ctime>

// Returns Content-Length value from HTTP headers, or 0 if not found
size_t get_content_length(const std::string& raw_headers);

// Returns true if the HTTP request is complete (headers and body)
bool is_request_complete(const std::string& buffer);

Response health();

Response echo(const std::string& msg);