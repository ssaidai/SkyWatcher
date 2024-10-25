#include "Logger.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <regex>
#include <algorithm>

// Static variables for log files and mutexes
static std::ofstream commandLogFile;
static std::ofstream visitLogFile;
static std::mutex commandLogMutex;
static std::mutex visitLogMutex;

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
void openLogFiles(const std::string& commandFilename, const std::string& visitFilename) {
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
void closeLogFiles() {
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
void logMessage(const std::string& subject, const std::string& type, const std::string& message) {
    std::lock_guard<std::mutex> lock(commandLogMutex);
    if (commandLogFile.is_open()) {
        commandLogFile << getCurrentTime() << " [" << type << "] " << subject << ": " << message << std::endl;
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

void logVisit(const std::string& droneID, double x, double y, double batteryLevel) {
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

bool parseVisitLogLine(const std::string& line, std::string& timestamp, std::string& droneID, double& x, double& y, double& batteryLevel) {
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
                std::cerr << "Error parsing numeric values in log line: " << line << std::endl;
                return false;
            }

            // Check if x and y are within acceptable ranges (0 to 6000 meters)
            if (x < 0.0 || x >= 6000.0 || y < 0.0 || y >= 6000.0) {
                std::cerr << "Error: Position out of bounds (" << x << ", " << y << ") in log line: " << line << std::endl;
                // Continue processing to collect all errors
            }

            // Check if batteryLevel is within acceptable range (0% < batteryLevel ≤ 100%)
            if (batteryLevel <= 0.0 || batteryLevel > 100.0) {
                std::cerr << "Error: Battery level out of bounds (" << batteryLevel << "%) in log line: " << line << std::endl;
                // Continue processing to collect all errors
            }

            return true;
        }
    }
    std::cerr << "Failed to parse log line: " << line << std::endl;
    return false;
}

void parseVisitLogFile(const std::string& visitLogFilename,
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

            // Map coordinates to cell indices (assuming cell size is 20m)
            int cellX = static_cast<int>(x / 20.0);
            int cellY = static_cast<int>(y / 20.0);

            // Ensure indices are within bounds (0 to 299)
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
        }
    }

    visitLogFile.close();

    // Sort visit times for each cell
    for (auto& cellEntry : cellVisitTimes) {
        std::sort(cellEntry.second.begin(), cellEntry.second.end());
    }
}

void analyzeCellVisits(const std::map<std::pair<int, int>, std::vector<std::chrono::system_clock::time_point>>& cellVisitTimes,
                       const std::chrono::system_clock::time_point& simulationStartTime,
                       const std::chrono::system_clock::time_point& simulationEndTime,
                       const std::chrono::minutes& maxInterval) {
    bool allCellsOk = true;

    // Iterate over all possible cells in the grid
    for (int x = 0; x < 20; ++x) {  // Adjusted to 300 to cover the entire grid
        for (int y = 0; y < 20; ++y) {
            std::pair<int, int> cellCoord = std::make_pair(x, y);
            auto it = cellVisitTimes.find(cellCoord);

            if (it == cellVisitTimes.end()) {
                // Cell was never visited
                allCellsOk = false;
                std::cout << "Cell (" << x << ", " << y << ") was never visited during the simulation." << std::endl;
            } else {
                const auto& visits = it->second;

                // Check from simulation start to first visit
                auto interval = std::chrono::duration_cast<std::chrono::minutes>(visits.front() - simulationStartTime);
                if (interval >= maxInterval) {
                    allCellsOk = false;
                    std::cout << "Cell (" << x << ", " << y << ") was not visited within the first " << maxInterval.count() << " minutes." << std::endl;
                    std::cout << "First visit at time: " << formatTimePoint(visits.front()) << std::endl;
                }

                // Check intervals between visits
                for (size_t i = 1; i < visits.size(); ++i) {
                    interval = std::chrono::duration_cast<std::chrono::minutes>(visits[i] - visits[i - 1]);
                    if (interval >= maxInterval) {
                        allCellsOk = false;
                        std::cout << "Cell (" << x << ", " << y << ") was not visited for " << interval.count() << " minutes between " << formatTimePoint(visits[i - 1]) << " and " << formatTimePoint(visits[i]) << "." << std::endl;
                    }
                }

                // Check from last visit to simulation end
                interval = std::chrono::duration_cast<std::chrono::minutes>(simulationEndTime - visits.back());
                if (interval >= maxInterval) {
                    allCellsOk = false;
                    std::cout << "Cell (" << x << ", " << y << ") was not visited in the last " << maxInterval.count() << " minutes before simulation end." << std::endl;
                    std::cout << "Last visit at: " << formatTimePoint(visits.back()) << std::endl;
                }
            }
        }
    }

    if (allCellsOk) {
        std::cout << "All cells were visited within every " << maxInterval.count() << "-minute interval." << std::endl;
    }
}

std::string formatTimePoint(const std::chrono::system_clock::time_point& timePoint) {
    std::time_t timeT = std::chrono::system_clock::to_time_t(timePoint);
    std::tm tm = *std::localtime(&timeT);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return buffer;
}
