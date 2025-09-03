#pragma once
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <cctype>

/**
 * @brief Represents an HTTP request and provides utilities for parsing and accessing its components.
 * @details Parses the raw HTTP request string into method, path, version, headers, and body.
 */
class Request {
public:
    // HTTP method (GET, POST, etc.)
    std::string method;
    // Request path (e.g., /index)
    std::string path;
    // HTTP version (e.g., HTTP/1.1)
    std::string version;
    // Query string (e.g., key=value&foo=bar)
    std::string query;
    // Map of header fields
    std::map<std::string, std::string> headers;
    // Request body
    std::string body;

	// Constructs a Request from a raw HTTP request string
    Request(const std::string& raw);

	// Gets the value of a query parameter by key
    std::string getQparams(const std::string& key) const;
};

