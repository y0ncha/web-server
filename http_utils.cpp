#include "http_utils.h"


/**
 * @brief Handles GET requests by dispatching to the appropriate handler based on the path.
 * @param request HTTP request
 * @return HTTP response
 */
Response handleGet(const Request& request) {

	if (request.path == "/health") { // Health check endpoint
        return health();
    } 
	else { // Fallback to static HTML file serving (return 404 if not found)
        return fetchHtmlFile(request);
    } 
}

Response handlePost(const Request& request) {
    if (request.path == "/echo") { // Echo endpoint
        return echo(request);
    } 
    else {
        return handleNotFound("Unsupported POST endpoint");
    }
}

/**
 * @brief Handles GET /health endpoint.
 * @return Plain text health check response
 */
Response health() {
    Response response = Response::ok("Computer Networks Web Server Assignment");
    response.headers["Content-Type"] = "text/plain";
    return response;
}

/**
 * @brief Handles GET /echo?msg=... endpoint.
 * @param request HTTP request
 * @return Echoed message or bad request response if missing
 */
Response echo(const Request& request) {

	std::string message = request.body;
    Response response;

    std::cout << "[POST] Received body: \"" << request.body << "\"\n";
    response = Response::ok(message);

    return response;
}

/**
 * @brief Resolves the file path for static HTML serving based on path and language.
 * @param path Request path
 * @param lang Language code
 * @return Resolved file path or empty string if not found
 */
std::string resolveFilePath(const std::string& path, const std::string& lang) {
    std::string baseName = path == "/" ? "index" : path.substr(1); // remove leading '/'
    std::string dir = "C:\\temp\\";
    std::string filePath;

    // Try lang-specific file
    if (!lang.empty()) {
        filePath = dir + baseName + "." + lang + ".html";
        std::ifstream fileLang(filePath);
        if (fileLang.good()) {
            return filePath;
        }
    }
    // Fallback to English
    filePath = dir + baseName + ".en.html";
    std::ifstream fileEn(filePath);
    if (fileEn.good()) {
        return filePath;
    }
    // Fallback to generic
    filePath = dir + baseName + ".html";
    std::ifstream fileGeneric(filePath);
    if (fileGeneric.good()) {
        return filePath;
    }
    return "";
}

/**
 * @brief Handles GET for HTML file endpoints (/, /about, /faq, etc.).
 * @param request HTTP request
 * @return File contents or not found response
 */
Response fetchHtmlFile(const Request& request) {
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
 * @brief Returns a 404 Not Found response, optionally with an error message.
 * @param error Error message
 * @return Not found response
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
 * @brief Returns a 400 Bad Request response, optionally with an error message.
 * @param error Error message
 * @return Bad request response
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
 * @brief Extracts Content-Length value from HTTP headers, or returns 0 if not found or invalid.
 * @param rawHeaders Raw HTTP headers string
 * @return Content-Length value
 */
size_t getContentLength(const std::string& rawHeaders) {
    size_t clPos = rawHeaders.find("Content-Length:");
    if (clPos == std::string::npos) {
        return 0;
    }
    size_t clEnd = rawHeaders.find("\r\n", clPos);
    if (clEnd == std::string::npos) {
        return 0;
    }
    std::string clVal = rawHeaders.substr(clPos + 15, clEnd - (clPos + 15));
    clVal.erase(std::remove_if(clVal.begin(), clVal.end(), ::isspace), clVal.end());
    try {
        return std::stoul(clVal);
    } catch (...) {
        return 0;
    }
}

/**
 * @brief Checks if the HTTP request in buffer is complete (headers and body).
 * @param buffer Raw HTTP request buffer
 * @return True if request is complete, false otherwise
 */
bool isRequestComplete(const std::string& buffer) {
    size_t headerEnd = buffer.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return false;
    }
    size_t contentLength = getContentLength(buffer.substr(0, headerEnd));
    size_t bodyStart = headerEnd + 4;
    size_t bodyLen = buffer.size() - bodyStart;
    if ((contentLength == 0 && bodyLen == 0) || (contentLength > 0 && bodyLen >= contentLength)) {
        return true;
    }
    return false;
}

/**
 * @brief Checks if the connection should be kept alive based on headers and HTTP version.
 * @param request HTTP request
 * @return True if keep-alive, false otherwise
 */
bool isKeepAlive(const Request& request) {
    auto connIt = request.headers.find("Connection");
    if (connIt != request.headers.end()) {
        std::string connVal = connIt->second;
        std::transform(connVal.begin(), connVal.end(), connVal.begin(), ::tolower);
        if (connVal == "keep-alive") {
            return true;
        }
        if (connVal == "close") {
            return false;
        }
    }
    // Default to HTTP/1.1 keep-alive, HTTP/1.0 close
    return request.version == "HTTP/1.1";
}