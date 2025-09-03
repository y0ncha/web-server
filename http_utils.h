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

// Handles HEAD requests by fetching the HTML file and returning headers only.
Response handleHead(const Request& request);

// Handles PUT requests. Stores body in file derived from path, returns appropriate response.
Response handlePut(const Request& request);

// Handles POST requests. Echoes body and prints to console.
Response handlePost(const Request& request);

// Handles DELETE requests. Deletes the .txt file derived from the path.
Response handleDelete(const Request& request);

// Handles TRACE requests. Currently not supported, returns 400 Bad Request.
Response handleTrace(const Request& request);

// Handles GET /health endpoint. Returns a plain text health check response.
Response health();

// Resolves the file path for static HTML serving based on path and language.
std::string resolveFilePath(const std::string& path, const std::string& lang);

// Checks if the HTTP request in buffer is complete (headers and body).
bool isRequestComplete(const std::string& buffer);

// Checks if the connection should be kept alive based on headers and HTTP version.
bool isKeepAlive(const Request& request);

// Extracts Content-Length value from HTTP headers, or returns 0 if not found or invalid.
size_t getContentLength(const std::string& rawHeaders);

// Returns a 404 Not Found response with a context-aware message.
Response handleNotFound(const std::string& context);

// Returns a 400 Bad Request response with a context-aware message.
Response handleBadRequest(const std::string& context);

// Returns a 201 Created response with a context-aware message.
Response handleCreated(const std::string& context);

// Returns a 200 OK response with a context-aware message.
Response handleOk(const std::string& context);

// Returns a 500 Internal Server Error response with a context-aware message.
Response handleInternalError(const std::string& context);