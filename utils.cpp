#include "utils.h"
#include <fstream>
#include <ctime>

/**
 * @brief Returns the current timestamp as a formatted string.
 * @return Timestamp string in format YYYY-MM-DD HH:MM:SS
 */
std::string getTimestamp() {
    std::time_t now = std::time(nullptr);
    char timeBuf[32];
    tm timeInfo;
    localtime_s(&timeInfo, &now);
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &timeInfo);
    return std::string(timeBuf);
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
 */
void logError(const std::string& message, int wsaError) {
    std::string errorMsg = "[" + getTimestamp() + "] Web Server: " + message;
    if (wsaError != -1) {
        errorMsg += " Error: " + std::to_string(wsaError);
    }
    std::ofstream logFile("web-server-error.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << errorMsg << std::endl;
        logFile.close();
    }
}

/**
 * @brief Logs sent or received data with timestamp and client address to a file.
 * @param filename Log file name
 * @param clientAddr Client address string
 * @param data Data to log
 */
void logData(const std::string& filename, const std::string& clientAddr, const std::string& data) {
    std::ofstream logFile(filename, std::ios::app);
    if (logFile.is_open()) {
        logFile << "--------------------" << std::endl;
        logFile << "[" << getTimestamp() << "] [" << clientAddr << "]" << std::endl;
        logFile << data << std::endl;
        logFile << "--------------------" << std::endl;
        logFile.close();
    }
}

/**
 * @brief Logs client state transitions with timestamp and client address to a file.
 * @param clientAddr Client address string
 * @param oldState Previous state
 * @param newState New state
 */
void logClientState(const std::string& clientAddr, const std::string& oldState, const std::string& newState) {
    std::ofstream logFile("web-server-clientstate.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << "[" << getTimestamp() << "] [" << clientAddr << "] "
                << "State transition: " << oldState << " -> " << newState << std::endl;
        logFile.close();
    }
}

