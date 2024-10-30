#include "WatchZone.h"

int main() {
    // logOpen("tower - " + getCurrentTime() + ".log");
    std::string logFile = "tower.log"; // adjust the filename to be unique using timestamp di needed
    openLogFiles(logFile);
    logInfo("Tower", "Initializing...");
    WatchZone watchZone(800, 10);
    closeLogFiles();
}