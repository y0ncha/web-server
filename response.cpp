#include "response.h"

/**
 * @brief Constructs a Response with default values
 * @details Default is 200 OK with empty body
 */
Response::Response() : statusCode(200), statusMessage("OK"), body(""), bodyLength(0) {}

/**
 * @brief Creates a 200 OK response with body
 * @param body Response body
 * @return Response object
 */
Response Response::ok(const std::string& body) {
    Response response;
    response.statusCode = 200;
    response.statusMessage = "OK";
    response.body = body;
    response.bodyLength = body.size();
    response.headers["Content-Type"] = "text/plain";
    return response;
}

/**
 * @brief Creates a 404 Not Found response with body
 * @param body Response body
 * @return Response object
 */
Response Response::notFound(const std::string& body) {
    Response response;
    response.statusCode = 404;
    response.statusMessage = "Not Found";
    response.body = body;
    response.bodyLength = body.size();
    response.headers["Content-Type"] = "text/plain";
    return response;
}

/**
 * @brief Creates a 400 Bad Request response with body
 * @param body Response body
 * @return Response object
 */
Response Response::badRequest(const std::string& body) {
    Response response;
    response.statusCode = 400;
    response.statusMessage = "Bad Request";
    response.body = body;
    response.bodyLength = body.size();
    response.headers["Content-Type"] = "text/plain";
    return response;
}

/**
 * @brief Creates a 201 Created response with body
 * @param body Response body
 * @return Response object
 */
Response Response::created(const std::string& body) {
    Response response;
    response.statusCode = 201;
    response.statusMessage = "Created";
    response.body = body;
    response.bodyLength = body.size();
    response.headers["Content-Type"] = "text/plain";
    return response;
}

/**
 * @brief Creates a 500 Internal Server Error response with body
 * @param body Response body
 * @return Response object
 */
Response Response::internalError(const std::string& body) {
    Response response;
    response.statusCode = 500;
    response.statusMessage = "Internal Server Error";
    response.body = body;
    response.bodyLength = body.size();
    response.headers["Content-Type"] = "text/plain";
    return response;
}

/**
 * @brief Converts the response to a raw HTTP string
 * @return HTTP response string
 */
std::string Response::toString() const {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    for (const auto& header : headers) {
        ss << header.first << ": " << header.second << "\r\n";
    }
    ss << "Content-Length: " << bodyLength << "\r\n";
    ss << "\r\n";
    ss << body;
    return ss.str();
}