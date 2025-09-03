#include "request.h"
#include "utils.h"
#include <sstream>
#include <algorithm>
#include <cctype>

// Constructs a Request from a raw HTTP request string
Request::Request(const std::string& raw) {
    std::istringstream stream(raw);
    std::string line;

    // Parse request line
    if (!std::getline(stream, line) || line.empty()) return;
    std::istringstream reqLine(line);
    reqLine >> method >> path >> version;

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
    std::ostringstream bodyStream;
    while (std::getline(stream, line)) {
        if (!body.empty()) bodyStream << "\n";
        bodyStream << line;
    }
    body = bodyStream.str();
}

// Gets the value of a query parameter by key
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