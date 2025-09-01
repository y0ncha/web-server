#include "request.h"
#include "utils.h"
#include <sstream>
#include <algorithm>
#include <cctype>

Request::Request(const std::string& raw) {
    std::istringstream stream(raw);
    std::string line;

    // Parse request line
    if (!std::getline(stream, line) || line.empty()) return;
    std::istringstream req_line(line);
    req_line >> method >> path >> version;

    // Parse query string
    auto qpos = path.find('?');
    if (qpos != std::string::npos) {
        query = path.substr(qpos + 1);
        path = path.substr(0, qpos);
    }

    // Parse headers
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        auto colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = trim(line.substr(0, colon));
            std::string value = trim(line.substr(colon + 1));
            if (!value.empty() && value.back() == '\r') value.pop_back();
            headers[key] = value;
        }
    }

    // Parse body (if any)
    std::ostringstream body_stream;
    while (std::getline(stream, line)) {
        if (!body.empty()) body_stream << "\n";
        body_stream << line;
    }
    body = body_stream.str();
}

std::string Request::getQparams(const std::string& key) const {
    std::istringstream qs(query);
    std::string pair;
    while (std::getline(qs, pair, '&')) {
        auto eq = pair.find('=');
        if (eq != std::string::npos) {
            std::string k = pair.substr(0, eq);
            std::string v = pair.substr(eq + 1);
            if (k == key) return v;
        }
    }
    return "";
}