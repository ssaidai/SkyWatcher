#include "Cerebrum.h"

Path Cerebrum::getPathToSector(int sectorId) {
    int x = sectorId % 30;
    int y = sectorId / 30;
    double distance;
    if (y < 15 && x < 15){
        distance = std::sqrt(std::pow(3000 - (200 * y + 190), 2) + std::pow(3000 - (200 * x + 190), 2));
    } else if (y < 15){
        x -= 15;
        distance = std::sqrt(std::pow(3000 - (200 * y + 190), 2) + std::pow(200 * x + 10, 2));
    } else if (x < 15){
        y -= 15;
        distance = std::sqrt(std::pow(200 * y + 10, 2) + std::pow(3000 - (200 * x + 190), 2));
    } else {
        x -= 15;
        y -= 15;
        distance = std::sqrt(std::pow(200 * y + 10, 2) + std::pow(200 * x + 10, 2));
    }

    return {distance, static_cast<float>(distance / (30/3.6))};
}

void Cerebrum::solveTSP() {
    // Populate the tsp paths for each region


}

void Cerebrum::initializeSectors(const std::vector<std::unique_ptr<Sector>> &sectors) {
    // Initialize paths to all sectors from the center
    for (int i = 0; i < sectors.size(); i++) {
        pathToSectors[i] = getPathToSector(i);
    }
}

