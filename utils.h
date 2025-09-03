#pragma once
#include <string>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <ctime>

// Returns the current timestamp as a formatted string
std::string getTimestamp();

// Trims whitespace from both ends of a string
std::string trim(const std::string& str);

// Logs an error message with timestamp to a file in the working directory
void logError(const std::string& message, int wsaError = -1);

// Logs sent or received data with timestamp and client address to a file
void logData(const std::string& filename, const std::string& clientAddr, const std::string& data);

// Logs client state transitions with timestamp
void logClientState(const std::string& clientAddr, const std::string& oldState, const std::string& newState);

// Validates and sanitizes PUT path, returns true if valid and sets baseName
bool isValidPutPath(const std::string& path, std::string& baseName);