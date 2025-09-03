#pragma once
#include "response.h"
#include "request.h"
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>

// Handles GET requests by dispatching to the appropriate handler based on the path.
Response handleGet(const Request& request);

// Handles POST requests. Currently only supports /echo endpoint.
Response handlePost(const Request& request);

// Handles GET /health endpoint. Returns a plain text health check response.
Response health();

// Handles GET /echo?msg=... endpoint. Returns the echoed message or a bad request response if missing.
Response echo(const Request& request);

// Handles GET for HTML file endpoints (/, /about, /faq, etc.). Returns the file contents or a not found response.
Response fetchHtmlFile(const Request& request);

// Returns a 404 Not Found response, optionally with an error message.
Response handleNotFound(const std::string& error = "");

// Returns a 400 Bad Request response, optionally with an error message.
Response handleBadRequest(const std::string& error = "");

// Resolves the file path for static HTML serving based on path and language.
std::string resolveFilePath(const std::string& path, const std::string& lang);

// Checks if the HTTP request in buffer is complete (headers and body).
bool isRequestComplete(const std::string& buffer);

// Checks if the connection should be kept alive based on headers and HTTP version.
bool isKeepAlive(const Request& request);

// Extracts Content-Length value from HTTP headers, or returns 0 if not found or invalid.
size_t getContentLength(const std::string& rawHeaders);