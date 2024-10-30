#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <chrono>
#include <mutex>

// Opens the log files
void openLogFiles(const std::string& logFilename);

// Closes the log files
void closeLogFiles();

// General log function for command logs
void logMessage(const std::string& subject, const std::string& type, const std::string& message);

// Specific logging functions for command logs
void logInfo(const std::string& subject, const std::string& message);
void logError(const std::string& subject, const std::string& message);
void logWarning(const std::string& subject, const std::string& message);
void logDebug(const std::string& subject, const std::string& message);
void logVisit(const std::string& subject, const std::string& message, const std::string& timestamp);

// Utility function to get current time
std::string getCurrentTime();

#endif // LOGGER_H