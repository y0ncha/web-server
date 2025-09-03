#pragma once
#include <string>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <ctime>

/**
 * @brief Returns the current timestamp as a formatted string.
 * @return Timestamp string in format YYYY-MM-DD HH:MM:SS
 */
std::string getTimestamp();

/**
 * @brief Trims whitespace from both ends of a string
 * @param str The string to trim
 * @return Trimmed string
 */
std::string trim(const std::string& str);

/**
 * @brief Logs an error message with timestamp to a file in the working directory.
 * @param message Error message
 * @param wsaError Optional WSA error code (default -1 means not provided)
 */
void logError(const std::string& message, int wsaError = -1);

/**
 * @brief Logs sent or received data with timestamp and client address to a file.
 * @param filename Log file name
 * @param clientAddr Client address string
 * @param data Data to log
 */
void logData(const std::string& filename, const std::string& clientAddr, const std::string& data);

/**
 * @brief Logs client state transitions with timestamp and client address to a file.
 * @param clientAddr Client address string
 * @param oldState Previous state
 * @param newState New state
 */
void logClientState(const std::string& clientAddr, const std::string& oldState, const std::string& newState);
