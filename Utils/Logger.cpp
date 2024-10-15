#include "Logger.h"
#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <regex>
#include <map>
#include <vector>
#include <algorithm>

Logger::Logger() {
    // Constructor
}

Logger::~Logger() {
    closeLogFiles();
}

// Function to get the current time as a formatted string
std::string Logger::getCurrentTime() {
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
void Logger::openLogFiles(const std::string& commandFilename, const std::string& visitFilename) {
    {
        std::lock_guard<std::mutex> lock(commandLogMutex);
        commandLogFile.open(commandFilename, std::ios_base::app); // Open in append mode
        if (!commandLogFile.is_open()) {
            std::cerr << "Failed to open command log file: " << commandFilename << std::endl;
        }
    }
    {
        std::lock_guard<std::mutex> lock(visitLogMutex);
        visitLogFile.open(visitFilename, std::ios_base::app); // Open in append mode
        if (!visitLogFile.is_open()) {
            std::cerr << "Failed to open visit log file: " << visitFilename << std::endl;
        }
    }
}

// Closes the log files
void Logger::closeLogFiles() {
    {
        std::lock_guard<std::mutex> lock(commandLogMutex);
        if (commandLogFile.is_open()) {
            commandLogFile.close();
        }
    }
    {
        std::lock_guard<std::mutex> lock(visitLogMutex);
        if (visitLogFile.is_open()) {
            visitLogFile.close();
        }
    }
}

// General log function for command logs
void Logger::log(const std::string& subject, const std::string& type, const std::string& message) {
    std::lock_guard<std::mutex> lock(commandLogMutex);
    if (commandLogFile.is_open()) {
        commandLogFile << getCurrentTime() << " [" << type << "] " << subject << ": " << message << std::endl;
    } else {
        std::cerr << "Command log file is not open!" << std::endl;
    }
}

// Specific logging functions for command logs
void Logger::logInfo(const std::string& subject, const std::string& message) {
    log(subject, "INFO", message);
}

void Logger::logError(const std::string& subject, const std::string& message) {
    log(subject, "ERROR", message);
}

void Logger::logWarning(const std::string& subject, const std::string& message) {
    log(subject, "WARNING", message);
}

void Logger::logDebug(const std::string& subject, const std::string& message) {
    log(subject, "DEBUG", message);
}

void Logger::logVisit(const std::string& droneID, double x, double y, double batteryLevel) {
    std::lock_guard<std::mutex> lock(visitLogMutex);
    if (visitLogFile.is_open()) {
        // Get the current time as a formatted string
        std::string visitTimeStr = getCurrentTime();

        // Log the visit message with the current time
        visitLogFile << visitTimeStr << " Drone " << droneID << " visited point "
                     << x << "," << y << " battery " << batteryLevel << std::endl;
    } else {
        std::cerr << "Visit log file is not open!" << std::endl;
    }
}

// Parsing function for visit logs
bool Logger::parseVisitLogLine(const std::string& line, std::string& timestamp, std::string& droneID, double& x, double& y, double& batteryLevel) {
    // Expected log line format:
    // YYYY-MM-DD HH:MM:SS Drone <DroneID> visited point <x>,<y> battery <batteryLevel>

    std::regex logEntryRegex(R"(^(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}) Drone (\S+) visited point (\S+),(\S+) battery (\S+)$)");
    std::smatch match;

    if (std::regex_match(line, match, logEntryRegex)) {
        if (match.size() == 6) {
            timestamp = match[1];
            droneID = match[2];
            try {
                x = std::stod(match[3]);
                y = std::stod(match[4]);
                batteryLevel = std::stod(match[5]);
            } catch (...) {
                // Parsing error
                return false;
            }
            return true;
        }
    }
    return false;
}

// Function to parse the entire visit log file
void Logger::parseVisitLogFile(const std::string& visitLogFilename,
                               std::map<std::pair<int, int>, std::vector<std::chrono::system_clock::time_point>>& cellVisitTimes,
                               std::chrono::system_clock::time_point& simulationStartTime,
                               std::chrono::system_clock::time_point& simulationEndTime) {
    std::ifstream visitLogFile(visitLogFilename);
    if (!visitLogFile.is_open()) {
        std::cerr << "Failed to open visit log file: " << visitLogFilename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(visitLogFile, line)) {
        std::string timestampStr;
        std::string droneID;
        double x, y, batteryLevel;

        if (parseVisitLogLine(line, timestampStr, droneID, x, y, batteryLevel)) {
            // Convert timestamp string to time_point
            std::tm tm = {};
            std::istringstream ss(timestampStr);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            if (ss.fail()) {
                std::cerr << "Failed to parse timestamp: " << timestampStr << std::endl;
                continue;
            }
            std::time_t timeT = std::mktime(&tm);
            std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::from_time_t(timeT);

            // Map coordinates to cell indices
            int cellX = static_cast<int>(x / 20.0); // Cell size is 20m
            int cellY = static_cast<int>(y / 20.0);

            // Ensure indices are within bounds
            if (cellX >= 0 && cellX < 300 && cellY >= 0 && cellY < 300) {
                std::pair<int, int> cellCoord = std::make_pair(cellX, cellY);
                cellVisitTimes[cellCoord].push_back(timestamp);

                // Update simulation start and end times
                if (simulationStartTime == std::chrono::system_clock::time_point::min() || timestamp < simulationStartTime) {
                    simulationStartTime = timestamp;
                }
                if (simulationEndTime == std::chrono::system_clock::time_point::min() || timestamp > simulationEndTime) {
                    simulationEndTime = timestamp;
                }
            } else {
                std::cerr << "Invalid cell coordinates: (" << cellX << ", " << cellY << ") for position (" << x << ", " << y << ")" << std::endl;
            }
        } else {
            std::cerr << "Failed to parse log line: " << line << std::endl;
        }
    }

    visitLogFile.close();

    // Sort visit times for each cell
    for (auto& cellEntry : cellVisitTimes) {
        std::sort(cellEntry.second.begin(), cellEntry.second.end());
    }
}

// Function to analyze cell visits
void Logger::analyzeCellVisits(const std::map<std::pair<int, int>, std::vector<std::chrono::system_clock::time_point>>& cellVisitTimes,
                               const std::chrono::system_clock::time_point& simulationStartTime,
                               const std::chrono::system_clock::time_point& simulationEndTime,
                               const std::chrono::minutes& maxInterval) {
    std::vector<std::pair<int, int>> cellsWithIssues;

    for (int x = 0; x < 4; ++x) {
        for (int y = 0; y < 4; ++y) {
            std::pair<int, int> cellCoord = std::make_pair(x, y);
            auto it = cellVisitTimes.find(cellCoord);
            bool cellHasIssue = false;

            if (it == cellVisitTimes.end()) {
                // Cell was never visited
                cellHasIssue = true;
            } else {
                const auto& visits = it->second;
                // Check from simulation start to first visit
                if ((visits.front() - simulationStartTime) >= maxInterval) {
                    cellHasIssue = true;
                }

                // Check intervals between visits
                for (size_t i = 1; i < visits.size(); ++i) {
                    auto duration = std::chrono::duration_cast<std::chrono::minutes>(visits[i] - visits[i - 1]);
                    if (duration >= maxInterval) {
                        cellHasIssue = true;
                        break; // No need to check further
                    }
                }

                // Check from last visit to simulation end, not sure about this
                if ((simulationEndTime - visits.back()) >= maxInterval) {
                    cellHasIssue = true;
                }
            }

            if (cellHasIssue) {
                cellsWithIssues.push_back(cellCoord);
            }
        }
    }

    // Report results
    if (cellsWithIssues.empty()) {
        std::cout << "All cells were visited within every " << maxInterval.count() << "-minute interval." << std::endl;
    } else {
        std::cout << "Cells that were not visited within every " << maxInterval.count() << "-minute interval:" << std::endl;
        for (const auto& cell : cellsWithIssues) {
            std::cout << "Cell (" << cell.first << ", " << cell.second << ")" << std::endl;
        }
        std::cout << "Total cells with issues: " << cellsWithIssues.size() << std::endl;
    }
}

/**
// Function to check the battery level of the drone
void checkBatteryLevel(const std::string& droneID, double batteryLevel) {
    if (batteryLevel <= 0.0) {
        Logger::logError(droneID, "Battery level is critically low or depleted.");
    } else if (batteryLevel <= 20.0) {
        Logger::logWarning(droneID, "Battery level is low.");
    } else {
        Logger::logInfo(droneID, "Battery level is sufficient.");
    }
}

// Function to check if the drone is within the designated area
void checkDronePosition(const std::string& droneID, double x, double y) {
    const double areaBoundary = 3000.0; // Since the area is 6000x6000 meters centered at (0,0)
    if (x < -areaBoundary || x > areaBoundary || y < -areaBoundary || y > areaBoundary) {
        Logger::logError(droneID, "Drone is exiting the designated area.");
    } else {
        Logger::logInfo(droneID, "Drone is within the designated area.");
    }
}
*/