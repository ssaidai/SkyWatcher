#include "Logger.h"  // Adjusted include to use the updated Logger header
#include <map>
#include <vector>
#include <chrono>
#include <iostream>

int main() {
    const std::string visitLogFilename = "visit.log";
    const std::chrono::minutes maxInterval(5);

    // Data structures to hold visit times and simulation time frame
    std::map<std::pair<int, int>, std::vector<std::chrono::system_clock::time_point>> cellVisitTimes;
    std::chrono::system_clock::time_point simulationStartTime = std::chrono::system_clock::time_point::min();
    std::chrono::system_clock::time_point simulationEndTime = std::chrono::system_clock::time_point::min();

    // Parse the visit log file using the free function
    parseVisitLogFile(visitLogFilename, cellVisitTimes, simulationStartTime, simulationEndTime);

    if (simulationStartTime == std::chrono::system_clock::time_point::min() ||
        simulationEndTime == std::chrono::system_clock::time_point::min()) {
        std::cerr << "No valid visit times found in the log file." << std::endl;
        return 1;
    }

    // Analyze the cell visits using the free function
    analyzeCellVisits(cellVisitTimes, simulationStartTime, simulationEndTime, maxInterval);

    return 0;
}
