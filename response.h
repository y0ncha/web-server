#pragma once
#include <string>
#include <map>
#include <sstream>

/**
 * @brief Represents an HTTP response and provides utilities for constructing and formatting it.
 * @details Manages status code, status message, headers, and body.
 */
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
    // Body length
    size_t bodyLength;

    // Constructs a Response with default values
    Response();

	// Factory methods for common responses
    // 
    // Creates a 200 OK response with body
    static Response ok(const std::string& body = "");
    // Creates a 404 Not Found response
    static Response notFound(const std::string& body = "");
    // Creates a 400 Bad Request response
    static Response badRequest(const std::string& body = "");
    // Creates a 201 Created response with body
    static Response created(const std::string& body = "");
    // Creates a 500 Internal Server Error response with body
    static Response internalError(const std::string& body = "");
    // Converts the response to a raw HTTP string


    std::string toString() const;
};