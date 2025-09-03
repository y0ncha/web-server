#pragma once
#include <string>
#include <map>
#include <sstream>

// Represents an HTTP response and provides utilities for HTTP formatting
class Response {
  public:
    // HTTP status code (e.g., 200, 404)
    int statusCode;
    // HTTP status message (e.g., OK, Not Found)
    std::string statusMessage;
    // Map of header fields
    std::map<std::string, std::string> headers;
    // Response body
    std::string body;

    // Constructs a Response with default values
    Response();
    // Creates a 200 OK response with body
    static Response ok(const std::string& body);
    // Creates a 404 Not Found response
    static Response not_found();
    // Creates a 400 Bad Request response
    static Response bad_request();
    // Converts the response to a raw HTTP string
    std::string toString() const;
};