#include "utils.h"
#include <algorithm>
#include <cctype>

// Trims whitespace from both ends of a string
std::string trim(const std::string& str) {
    auto begin = str.begin();
    while (begin != str.end() && std::isspace(*begin)) {
        ++begin;
    }
    auto end = str.end();
    do {
        --end;
    } while (std::distance(begin, end) > 0 && std::isspace(*end));
    return std::string(begin, end + 1);
}

