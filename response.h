#pragma once
#include <string>
#include <map>
#include <sstream>

class Response {
public:

    int status_code = 200;
    std::string status_text = "OK";
    std::string version = "HTTP/1.1";
    std::map<std::string, std::string> headers;
    std::string body;

    std::string toString() const;

    // Static shortcut helpers
    static Response not_found() {
        Response r;
        r.status_code = 404;
        r.status_text = "Not Found";
        r.body = "404 Not Found";
        r.headers["Content-Type"] = "text/plain";
        return r;
    }
    static Response bad_request() {
        Response r;
        r.status_code = 400;
        r.status_text = "Bad Request";
        r.body = "400 Bad Request";
        r.headers["Content-Type"] = "text/plain";
        return r;
    }
    static Response ok(const std::string& body = "") {
        Response r;
        r.status_code = 200;
        r.status_text = "OK";
        r.body = body;
        r.headers["Content-Type"] = "text/plain";
        return r;
    }
};