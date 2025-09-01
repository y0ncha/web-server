#include "response.h"

std::string Response::toString() const {
    std::ostringstream out;
    out << version << " " << status_code << " " << status_text << "\r\n";
    bool has_content_length = headers.find("Content-Length") != headers.end();
    if (!body.empty() && !has_content_length) {
        out << "Content-Length: " << body.size() << "\r\n";
    }
    for (const auto& h : headers) {
        out << h.first << ": " << h.second << "\r\n";
    }
    out << "\r\n";
    out << body;
    return out.str();
}