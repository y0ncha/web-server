#pragma once
#include "response.h"
#include "request.h"
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>

// Handler for GET /health
Response handle_health();
// Handler for GET /echo?msg=...
Response handle_echo(const Request& req);
// Handler for GET /, /about, /faq, etc. (serves HTML files)
Response handle_html_file(const Request& req);
// Fallback handler for 404 Not Found
Response handle_not_found(const std::string& error = "");
// Fallback handler for 400 Bad Request
Response handle_bad_request(const std::string& error = "");

// Helper: resolve file path for static HTML serving
std::string resolve_file_path(const std::string& path, const std::string& lang);

// Helper: check if HTTP request is complete
bool is_request_complete(const std::string& buffer);

// Helper: check if connection should be kept alive
bool is_keep_alive(const Request& req);

// Returns Content-Length value from HTTP headers, or 0 if not found
size_t get_content_length(const std::string& raw_headers);