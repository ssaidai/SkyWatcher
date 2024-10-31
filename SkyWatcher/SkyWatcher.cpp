#include "WatchZone.h"

int main(const int argc, char* argv[]) {
    // logOpen("tower - " + getCurrentTime() + ".log");
    const std::string logFile = "tower.log"; // adjust the filename to be unique using timestamp di needed
    openLogFiles(logFile);
    logInfo("Tower", "Initializing...");
    if (argc == 1 || argc > 4) {
        logError("Tower", "Invalid number of arguments. Usage: ./tower [areaSize] [timeScale]");
        return 1;
    }
    if (argc == 2) {
        logInfo("Tower", "Starting tower with area size: " + std::string(argv[1]) + " and default time scale: 10");

        WatchZone watchZone(std::stoi(argv[1]), 10);
    }
    if (argc == 3) {
        logInfo("Tower", "Starting tower with area size: " + std::string(argv[1]) + " and time scale: " + std::string(argv[2]));
        WatchZone watchZone(std::stoi(argv[1]), std::stoi(argv[2]));
    } else {
        logInfo("Tower", "Starting tower with default area size: 1200 and default time scale: 10");
        WatchZone watchZone(800, 10);
    }
    closeLogFiles();
}