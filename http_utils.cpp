#include "http_utils.h"

/**
 * Handles GET /health endpoint.
 * Returns a plain text health check response.
 */
Response handleHealth() {
    Response response = Response::ok("Computer Networks Web Server Assignment");
    response.headers["Content-Type"] = "text/plain";
    return response;
}

/**
 * Handles GET /echo?msg=... endpoint.
 * Returns the echoed message or a bad request response if missing.
 */
Response handleEcho(const Request& request) {
    std::string message = request.getQparams("msg");
    Response response;
    if (message.empty()) {
        response = handleBadRequest("Missing 'msg' query parameter");
        return response;
    }
    response = Response::ok(message);
    response.headers["Content-Type"] = "text/plain";
    return response;
}

/**
 * Resolves the file path for static HTML serving based on path and language.
 */
std::string resolveFilePath(const std::string& path, const std::string& lang) {
    std::string baseName = path == "/" ? "index" : path.substr(1); // remove leading '/'
    std::string dir = "C:\\temp\\";
    std::string filePath;

    // Try lang-specific file
    if (!lang.empty()) {
        filePath = dir + baseName + "." + lang + ".html";
        std::ifstream fileLang(filePath);
        if (fileLang.good()) return filePath;
    }
    // Fallback to English
    filePath = dir + baseName + ".en.html";
    std::ifstream fileEn(filePath);
    if (fileEn.good()) return filePath;
    // Fallback to generic
    filePath = dir + baseName + ".html";
    std::ifstream fileGeneric(filePath);
    if (fileGeneric.good()) return filePath;
    return "";
}

/**
 * Handles GET for HTML file endpoints (/, /about, /faq, etc.).
 * Returns the file contents or a not found response.
 */
Response handleHtmlFile(const Request& request) {
    std::string lang = request.getQparams("lang");
    std::string filePath = resolveFilePath(request.path, lang);
    if (filePath.empty()) {
        std::ostringstream ss;
        ss << "File not found for path " << request.path;
        return handleNotFound(ss.str());
    }
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::ostringstream ss;
        ss << "File could not be opened " << filePath;
        return handleNotFound(ss.str());
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    Response response = Response::ok(ss.str());
    response.headers["Content-Type"] = "text/html";
    return response;
}

/**
 * Returns a 404 Not Found response, optionally with an error message.
 */
Response handleNotFound(const std::string& error) {
    Response response = Response::not_found();
    if (!error.empty()) {
        response.body += " : ";
        response.body += error;
    }
    return response;
}

/**
 * Returns a 400 Bad Request response, optionally with an error message.
 */
Response handleBadRequest(const std::string& error) {
    Response response = Response::bad_request();
    if (!error.empty()) {
        response.body += " : ";
        response.body += error;
    }
    return response;
}

/**
 * Extracts Content-Length value from HTTP headers, or returns 0 if not found or invalid.
 */
size_t getContentLength(const std::string& rawHeaders) {
    size_t clPos = rawHeaders.find("Content-Length:");
    if (clPos == std::string::npos) return 0;
    size_t clEnd = rawHeaders.find("\r\n", clPos);
    if (clEnd == std::string::npos) return 0;
    std::string clVal = rawHeaders.substr(clPos + 15, clEnd - (clPos + 15));
    clVal.erase(std::remove_if(clVal.begin(), clVal.end(), ::isspace), clVal.end());
    try {
        return std::stoul(clVal);
    }
    catch (...) {
        return 0;
    }
}

/**
 * Checks if the HTTP request in buffer is complete (headers and body).
 */
bool isRequestComplete(const std::string& buffer) {
    size_t headerEnd = buffer.find("\r\n\r\n");
    if (headerEnd == std::string::npos) return false;
    size_t contentLength = getContentLength(buffer.substr(0, headerEnd));
    size_t bodyStart = headerEnd + 4;
    size_t bodyLen = buffer.size() - bodyStart;
    if ((contentLength == 0 && bodyLen == 0) || (contentLength > 0 && bodyLen >= contentLength)) {
        return true;
    }
    return false;
}

/**
 * Checks if the connection should be kept alive based on headers and HTTP version.
 */
bool isKeepAlive(const Request& request) {
    auto connIt = request.headers.find("Connection");
    if (connIt != request.headers.end()) {
        std::string connVal = connIt->second;
        std::transform(connVal.begin(), connVal.end(), connVal.begin(), ::tolower);
        if (connVal == "keep-alive") return true;
        if (connVal == "close") return false;
    }
    // Default to HTTP/1.1 keep-alive, HTTP/1.0 close
    return request.version == "HTTP/1.1";
}