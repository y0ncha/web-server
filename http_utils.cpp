#include "http_utils.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

Response handle_health() {
    Response res = Response::ok("Computer Networks Web Server Assignment");
    res.headers["Content-Type"] = "text/plain";
    return res;
}

Response handle_echo(const Request& req) {
    std::string msg = req.getQparams("msg");
    if (msg.empty()) {
        Response res = Response::bad_request();
        res.body = "Missing 'msg' query parameter";
        res.headers["Content-Type"] = "text/plain";
        return res;
    }
    Response res = Response::ok(msg);
    res.headers["Content-Type"] = "text/plain";
    return res;
}

std::string resolve_file_path(const std::string& path, const std::string& lang) {
    std::string base_name = path == "/" ? "index" : path.substr(1); // remove leading '/'
    std::string dir = "C:\\temp\\";
    std::string file_path;

    // Try lang-specific file
    if (!lang.empty()) {
        file_path = dir + base_name + "." + lang + ".html";
        std::ifstream f1(file_path);
        if (f1.good()) return file_path;
    }
    // Fallback to English
    file_path = dir + base_name + ".en.html";
    std::ifstream f2(file_path);
    if (f2.good()) return file_path;
    // Fallback to generic
    file_path = dir + base_name + ".html";
    std::ifstream f3(file_path);
    if (f3.good()) return file_path;
    return "";
}

Response handle_html_file(const Request& req) {
    std::string lang = req.getQparams("lang");
    std::string file_path = resolve_file_path(req.path, lang);
    if (file_path.empty()) {
        std::ostringstream ss;
        ss << "File not found for path " << req.path;
        return handle_not_found(ss.str());
    }
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::ostringstream ss;
        ss << "File could not be opened " << file_path;
        return handle_not_found(ss.str());
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    Response res = Response::ok(ss.str());
    res.headers["Content-Type"] = "text/html";
    return res;
}

Response handle_not_found(const std::string& error) {
    Response res = Response::not_found();
    if (!error.empty()) {
        res.body += " : ";
        res.body += error;
    }
    return res;
}

Response handle_bad_request(const std::string& error) {
    Response res = Response::bad_request();
    if (!error.empty()) {
        res.body += " : ";
        res.body += error;
    }
	return res;
}

size_t get_content_length(const std::string& raw_headers) {
    size_t cl_pos = raw_headers.find("Content-Length:");
    if (cl_pos == std::string::npos) return 0;
    size_t cl_end = raw_headers.find("\r\n", cl_pos);
    if (cl_end == std::string::npos) return 0;
    std::string cl_val = raw_headers.substr(cl_pos + 15, cl_end - (cl_pos + 15));
    cl_val.erase(std::remove_if(cl_val.begin(), cl_val.end(), ::isspace), cl_val.end());
    try {
        return std::stoul(cl_val);
    }
    catch (...) {
        return 0;
    }
}

bool is_request_complete(const std::string& buffer) {
    size_t header_end = buffer.find("\r\n\r\n"); // FIXED
    if (header_end == std::string::npos) return false;
    size_t content_length = get_content_length(buffer.substr(0, header_end));
    size_t body_start = header_end + 4;
    size_t body_len = buffer.size() - body_start;
    if ((content_length == 0 && body_len == 0) || (content_length > 0 && body_len >= content_length)) {
        return true;
    }
    return false;
}

bool is_keep_alive(const Request& req) {
    auto conn_it = req.headers.find("Connection");
    if (conn_it != req.headers.end()) {
        std::string conn_val = conn_it->second;
        std::transform(conn_val.begin(), conn_val.end(), conn_val.begin(), ::tolower);
        if (conn_val == "keep-alive") return true;
        if (conn_val == "close") return false;
    }
    // Default to HTTP/1.1 keep-alive, HTTP/1.0 close
    return req.version == "HTTP/1.1";
}