#include "http_utils.h"
#include "response.h"

/**
 * @brief Handles GET requests for files with language support and /health endpoint.
 * @param request HTTP request
 * @return HTTP response
 */
Response handleGet(const Request& request) {
    if (request.path == "/health") {
        return health();
    }
    std::string lang = request.getQparams("lang");
    std::string filePath = resolveFilePath(request.path, lang);
    if (filePath.empty()) {
        return handleNotFound(request.path);
    }
    std::ifstream infile(filePath);
    if (!infile.good()) {
        return handleNotFound(filePath);
    }
    std::ostringstream ss;
    ss << infile.rdbuf();
    infile.close();

    Response response = Response::ok(ss.str());
    // Set content type based on extension
    if (filePath.size() >= 5 && filePath.substr(filePath.size() - 5) == ".html") {
        response.headers["Content-Type"] = "text/html";
    } else {
        response.headers["Content-Type"] = "text/plain";
    }
    response.bodyLength = response.body.size();
    return response;
}

/**
 * @brief Handles POST requests. Only supports /echo endpoint.
 * @param request HTTP request
 * @return HTTP response
 */
Response handlePost(const Request& request) {
    // Validate Content-Type
    auto ctIt = request.headers.find("Content-Type");
    if (ctIt == request.headers.end() || ctIt->second != "text/plain") {
        return handleBadRequest("Unsupported Content-Type for POST. Only text/plain allowed.");
    }
    if (request.path != "/echo") {
        return handleBadRequest("POST only supported on /echo");
    }
    std::cout << "[POST] Received body: \"" << request.body << "\"\n";
    return Response::ok(request.body);
}

/**
 * @brief Handles HEAD requests for files with language support.
 * @param request HTTP request
 * @return HTTP response with headers only
 */
Response handleHead(const Request& request) {
    std::string lang = request.getQparams("lang");
    std::string filePath = resolveFilePath(request.path, lang);
    if (filePath.empty()) {
		return Response::notFound();
    }
    std::ifstream infile(filePath, std::ios::binary | std::ios::ate);
    if (!infile.good()) {
		return Response::notFound();
    }
    size_t fileSize = infile.tellg();
    infile.close();

    Response response = Response::ok("");
    // Set content type based on extension
    if (filePath.size() >= 5 && filePath.substr(filePath.size() - 5) == ".html") {
        response.headers["Content-Type"] = "text/html";
    } else {
        response.headers["Content-Type"] = "text/plain";
    }
    response.body.clear(); // No body for HEAD
    response.bodyLength = fileSize; // Set correct content length for header
    return response;
}

/**
 * @brief Handles PUT requests by writing the request body to a .txt file.
 * @param request HTTP request
 * @return HTTP response
 */
Response handlePut(const Request& request) {
    // Validate Content-Type
    auto ctIt = request.headers.find("Content-Type");
    if (ctIt == request.headers.end()) {
        return handleBadRequest("Missing Content-Type for PUT.");
    }

    std::string contentType = ctIt->second;
    std::string baseName;
    if (!isValidPutPath(request.path, baseName)) {
        return handleBadRequest("Invalid or missing path for PUT: " + request.path);
    }

    // Determine extension based on Content-Type
    std::string extension;
    if (contentType == "text/plain") {
        extension = ".txt";
    } else if (contentType == "text/html") {
        extension = ".html";
    } else {
        return handleBadRequest("Unsupported Content-Type for PUT. Only text/plain and text/html allowed.");
    }

    // Block index/about and index.lang/about.lang
    std::string lowerBase = baseName;
    std::transform(lowerBase.begin(), lowerBase.end(), lowerBase.begin(), ::tolower);
    if (lowerBase == "index" || lowerBase == "about") {
        return handleBadRequest("PUT not allowed for index or about.");
    }
    // Block index.lang/about.lang for .html files
    if (extension == ".html") {
        size_t dotPos = lowerBase.find('.');
        if (dotPos != std::string::npos) {
            std::string prefix = lowerBase.substr(0, dotPos);
            if (prefix == "index" || prefix == "about") {
                return handleBadRequest("PUT not allowed for index.lang or about.lang files.");
            }
        }
    }

    std::string filePath = "C:\\temp\\" + baseName + extension;
    bool fileExists = false;
    {
        std::ifstream infile(filePath);
        fileExists = infile.good();
    }
    try {
        std::ofstream outfile(filePath, std::ios::trunc);
        if (!outfile.is_open()) {
            return handleBadRequest("Could not open file for writing: " + filePath);
        }
        outfile << request.body;
        outfile.close();
    } catch (...) {
        return handleInternalError("Error writing file: " + filePath);
    }
    std::string fileName = filePath.substr(filePath.find_last_of("\\/") + 1);
    if (fileExists) {
        return handleOk(fileName);
    } else {
        return handleCreated(fileName);
    }
}

/**
 * @brief Handles DELETE requests by deleting the .txt file derived from the path.
 * @param request HTTP request
 * @return HTTP response
 */
Response handleDelete(const Request& request) {
    std::string baseName;
    if (!isValidPutPath(request.path, baseName)) {
        return handleBadRequest("Invalid or missing path for DELETE: " + request.path);
    }
    std::string filePath = "C:\\temp\\" + baseName + ".txt";
    std::ifstream infile(filePath);
    if (!infile.good()) {
        return handleNotFound(filePath);
    }
    infile.close();
    std::string fileName = filePath.substr(filePath.find_last_of("\\/") + 1);
    if (std::remove(filePath.c_str()) == 0) {
        return handleOk(fileName);
    } else {
        return handleInternalError("Error deleting file: " + fileName);
    }
}

/**
 * @brief Handles TRACE requests by echoing back the raw HTTP request string.
 * @param request HTTP request
 * @return HTTP response
 */
Response handleTrace(const Request& request) {
    std::ostringstream ss;
    for (const auto& header : request.headers) {
        ss << header.first << ": " << header.second << "\r\n";
    }
    ss << "\r\n" << request.body;
    Response response = Response::ok(ss.str());
    response.bodyLength = response.body.size();
    return response;
}

/**
 * @brief Handles OPTIONS requests. Returns allowed methods for the resource.
 * @param request HTTP request
 * @return HTTP response with Allow header
 */
Response handleOptions(const Request& request) {
    Response response = Response::ok("");
    response.headers["Allow"] = "GET, POST, PUT, DELETE, HEAD, TRACE, OPTIONS";
    response.bodyLength = 0;
    return response;
}

/**
 * @brief Handles GET /health endpoint.
 * @return Plain text health check response
 */
Response health() {
    Response response = Response::ok("Computer Networks Web Server Assignment");
    response.headers["Content-Type"] = "text/plain";
    response.bodyLength = response.body.size();
    return response;
}

/**
 * @brief Resolves the file path for static HTML or text serving based on path and language.
 * @param path Request path
 * @param lang Language code (e.g., "en", "fr")
 * @return Resolved file path or empty string if not found or invalid
 */
std::string resolveFilePath(const std::string& path, const std::string& lang) {
    std::string baseName;
    if (!isValidPutPath(path, baseName) && path != "/") {
        return "";
    }
    // Special case for root "/"
    if (path == "/") {
        baseName = "index";
    }

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
    // Fallback to generic HTML
    filePath = dir + baseName + ".html";
    std::ifstream fileGeneric(filePath);
    if (fileGeneric.good()) {
        return filePath;
    }
    // Fallback to .txt (for text file endpoints)
    filePath = dir + baseName + ".txt";
    std::ifstream fileTxt(filePath);
    if (fileTxt.good()) {
        return filePath;
    }
    return "";
}

/**
 * @brief Returns a 404 Not Found response, optionally with an error message.
 * @param error Error message
 * @return Not found response
 */
Response handleNotFound(const std::string& context) {
    return Response::notFound("Path not found: " + context);
}

/**
 * @brief Returns a 400 Bad Request response, optionally with an error message.
 * @param error Error message
 * @return Bad request response
 */
Response handleBadRequest(const std::string& context) {
    return Response::badRequest("Bad request: " + context);
}

/**
 * @brief Returns a 201 Created response, optionally with a location or message.
 * @param context Optional context message
 * @return Created response
 */
Response handleCreated(const std::string& context) {
    return Response::created("File created: " + context);
}

/**
 * @brief Returns a 200 OK response, optionally with a message.
 * @param context Optional context message
 * @return OK response
 */
Response handleOk(const std::string& context) {
    return Response::ok("File overwritten: " + context);
}

/**
 * @brief Returns a 500 Internal Server Error response, optionally with an error message.
 * @param error Error message
 * @return Internal error response
 */
Response handleInternalError(const std::string& context) {
    return Response::internalError("Internal error: " + context);
}

/**
 * @brief Extracts Content-Length value from HTTP headers, or returns 0 if not found or invalid.
 * @param rawHeaders Raw HTTP headers string
 * @return Content-Length value
 */
size_t getContentLength(const std::string& rawHeaders) {
    std::istringstream stream(rawHeaders);
    std::string line;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::string lowerLine = line;
        std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);

        if (lowerLine.rfind("content-length:", 0) == 0) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string clVal = line.substr(colonPos + 1);
                clVal.erase(std::remove_if(clVal.begin(), clVal.end(), ::isspace), clVal.end());
                try {
                    return std::stoul(clVal);
                }
                catch (...) {
                    return 0;
                }
            }
        }
    }
    return 0;
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
