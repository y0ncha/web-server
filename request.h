#pragma once
#include <string>
#include <map>

class Request {
public:
    std::string method;
    std::string path;
    std::string query;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;

    Request(const std::string& raw);
    std::string getQparams(const std::string& key) const;
};

