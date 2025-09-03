#include "response.h"
#include <sstream>

/**
 * @brief Constructs a Response with default values
 * @details Default is 200 OK with empty body
 */
Response::Response() : statusCode(200), statusMessage("OK"), body("") {}

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
    response.headers["Content-Type"] = "text/plain";
    return response;
}

/**
 * @brief Creates a 404 Not Found response
 * @return Response object
 */
Response Response::not_found() {
    Response response;
    response.statusCode = 404;
    response.statusMessage = "Not Found";
    response.body = "404 Not Found";
    response.headers["Content-Type"] = "text/plain";
    return response;
}

/**
 * @brief Creates a 400 Bad Request response
 * @return Response object
 */
Response Response::bad_request() {
    Response response;
    response.statusCode = 400;
    response.statusMessage = "Bad Request";
    response.body = "400 Bad Request";
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
    ss << "Content-Length: " << body.size() << "\r\n";
    ss << "\r\n";
    ss << body;
    return ss.str();
}