#include "utils.h"
#include <chrono>
#include <direct.h> // For _mkdir on Windows

void ensureLogDir() {
    _mkdir("log"); // Creates log directory if it doesn't exist
}

/**
 * @brief Returns the current timestamp as a formatted string with milliseconds.
 * @return Timestamp string in format YYYY-MM-DD HH:MM:SS.mmm
 */
std::string getTimestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t now_c = system_clock::to_time_t(now);
    tm timeInfo;
    localtime_s(&timeInfo, &now_c);

    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &timeInfo);

    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    char result[40];
    std::snprintf(result, sizeof(result), "%s.%03lld", timeBuf, static_cast<long long>(ms.count()));
    return std::string(result);
}

/**
 * @brief Trims whitespace from both ends of a string
 * @param str The string to trim
 * @return Trimmed string
 */
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

/**
 * @brief Logs an error message with timestamp to a file in the working directory.
 * @param message Error message
 * @param wsaError Optional WSA error code (default -1 means not provided)
 * @param port Optional port number related to the error (default -1 means not provided)
 */
void logError(const std::string& message, int errorCode, int port) {
    ensureLogDir();
    std::ofstream logFile("log/web-server-error.log", std::ios::app);
    logFile << "[" << getTimestamp() << "] ";
    logFile << message << " (WSAError: " << errorCode << ")";
    if (port != -1) {
        logFile << " [Port: " << port << "]";
    }
    logFile << std::endl;
}

/**
 * @brief Logs sent or received data with timestamp and client address to a file.
 * @param filename Log file name
 * @param clientAddr Client address string
 * @param data Data to log
 */
void logEvent(const std::string& filename, const std::string& clientAddr, const std::string& data) {
    ensureLogDir();
    std::ofstream logFile("log/" + filename, std::ios::app);
    if (logFile.is_open()) {
        logFile << "--------------------" << std::endl;
        logFile << "[" << getTimestamp() << "] [" << clientAddr << "]" << std::endl;
        logFile << data << std::endl;
        logFile << "--------------------" << std::endl;
        logFile.close();
    }
}

/**
 * @brief Logs sent or received data with timestamp and client address to a file.
 * @param filename Log file name
 * @param clientAddr Client address string
 * @param data Data to log
 */
void logData(const std::string& filename, const std::string& data) {
    ensureLogDir();
    std::ofstream logFile("log/" + filename, std::ios::app);
    if (logFile.is_open()) {
        logFile << data << std::endl;
    }
}

/**
 * @brief Logs client state transitions with timestamp and client address to a file.
 * @param clientAddr Client address string
 * @param oldState Previous state
 * @param newState New state
 */
void logClientState(const std::string& clientAddr, const std::string& oldState, const std::string& newState) {
    ensureLogDir();
    std::ofstream logFile("log/web-server-clientstate.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << "[" << getTimestamp() << "] [" << clientAddr << "] "
                << "State transition: " << oldState << " -> " << newState << std::endl;
        logFile.close();
    }
}

bool isValidPutPath(const std::string& path, std::string& baseName) {
    if (path.empty() || path[0] != '/' || path.size() < 2) {
        return false;
    }

    std::string candidate = path.substr(1); // remove leading '/'
    for (char c : candidate) {
        if (!std::isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }
    baseName = candidate;
    return true;
}