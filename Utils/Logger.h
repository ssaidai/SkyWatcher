#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <chrono>
#include <mutex>
#include <map>
#include <vector>

// Opens the log files
void openLogFiles(const std::string& commandFilename, const std::string& visitFilename);

// Closes the log files
void closeLogFiles();

// General log function for command logs
void logMessage(const std::string& subject, const std::string& type, const std::string& message);

// Specific logging functions for command logs
void logInfo(const std::string& subject, const std::string& message);
void logError(const std::string& subject, const std::string& message);
void logWarning(const std::string& subject, const std::string& message);
void logDebug(const std::string& subject, const std::string& message);

// Logging function for visit logs
void logVisit(const std::string& droneID, double x, double y, double batteryLevel);

// Utility function to get current time
std::string getCurrentTime();

// Parsing function for visit logs
bool parseVisitLogLine(const std::string& line, std::string& timestamp, std::string& droneID, double& x, double& y, double& batteryLevel);

// Functions to perform the checks
// Function to parse the entire visit log file and build visit times map
void parseVisitLogFile(const std::string& visitLogFilename,
                       std::map<std::pair<int, int>, std::vector<std::chrono::system_clock::time_point>>& cellVisitTimes,
                       std::chrono::system_clock::time_point& simulationStartTime,
                       std::chrono::system_clock::time_point& simulationEndTime);

// Function to analyze cell visits
void analyzeCellVisits(const std::map<std::pair<int, int>, std::vector<std::chrono::system_clock::time_point>>& cellVisitTimes,
                       const std::chrono::system_clock::time_point& simulationStartTime,
                       const std::chrono::system_clock::time_point& simulationEndTime,
                       const std::chrono::minutes& maxInterval);

// Function to format time points
std::string formatTimePoint(const std::chrono::system_clock::time_point& timePoint);

#endif // LOGGER_H