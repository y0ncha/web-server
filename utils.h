#pragma once
#include <algorithm>
#include <string>
#include <cctype>

inline std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) ++start;
    auto end = s.end();
    do { --end; } while (end != start && std::isspace(*end));
    return std::string(start, end + 1);
}
