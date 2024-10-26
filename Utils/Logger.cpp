#include "Logger.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>


// Static variables for log files and mutexes
static std::ofstream logFile;
static std::mutex logMutex;

// Function to get the current time as a formatted string
std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::ostringstream ss;
    tm buf;

    #if defined(_WIN32) || defined(_WIN64)
        // For Windows
        localtime_s(&buf, &in_time_t);
    #else
        // For Unix/Linux
        localtime_r(&in_time_t, &buf);
    #endif

    ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Opens the log files
void openLogFiles(const std::string& logFilename) {
    {
        std::lock_guard<std::mutex> lock(logMutex);
        logFile.open(logFilename, std::ios_base::app); // Open in append mode
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << logFilename << std::endl;
        }
    }
}

// Closes the log files
void closeLogFiles() {
    {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile.close();
        }
    }
}

// General log function for command logs
void logMessage(const std::string& subject, const std::string& type, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile << getCurrentTime() << " [" << type << "] " << subject << ": " << message << std::endl;
    } else {
        std::cerr << "Command log file is not open!" << std::endl;
    }
}

// Specific logging functions for command logs
void logInfo(const std::string& subject, const std::string& message) {
    logMessage(subject, "INFO", message);
}

void logError(const std::string& subject, const std::string& message) {
    logMessage(subject, "ERROR", message);
}

void logWarning(const std::string& subject, const std::string& message) {
    logMessage(subject, "WARNING", message);
}

void logDebug(const std::string& subject, const std::string& message) {
    logMessage(subject, "DEBUG", message);
}

