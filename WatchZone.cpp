#include "WatchZone.h"

WatchZone::WatchZone() {
    // Initialize sectors
    // Get sector number based on region area and cell size
    float cellSize = 20; // Assuming 20m x 20m cells
    int sectorSize = 10; // 10x10 cells per sector
    int numSectorsPerRow = width / (sectorSize * cellSize); // Calculate sectors per row

    // Initialize the whole grid first (could be done lazily if memory is a concern)
    std::vector<std::vector<Cell>> allCells(height / cellSize, std::vector<Cell>(width / cellSize));

    // Initialize cell boundaries (optional, depending on Cell constructor)
    for (int i = 0; i < height / cellSize; i++) {
        for (int j = 0; j < width / cellSize; j++) {
            allCells[i][j] = Cell(j * cellSize, (j + 1) * cellSize, i * cellSize, (i + 1) * cellSize);
        }
    }

    // Create sectors
    int sectorID = 0;
    for (int y = 0; y < height; y += sectorSize * cellSize) {
        for (int x = 0; x < width; x += sectorSize * cellSize) {
            sectors.push_back(std::make_unique<Sector>(sectorID++, x / cellSize, y / cellSize, allCells));
        }
    }

    // Initialize drones
    for (int i = 0; i < droneCount; i++) {
        drones.push_back(Drone());
    }

    // Initialize cerebrum and pass in the sectors
    this->cerebrum = Cerebrum();
    cerebrum.initializeSectors(sectors);

    // Initialize the TSP solver with the paths to all sectors
}

Position WatchZone::getDronePosition(int droneID) {
    return drones[droneID].getPosition();
}

// This function calculates the area size of the region that a single drone can cover in 5 minutes
double getRegionArea() {
    // Calculate the distance that a drone can cover in 5 minutes
    // double distance = Drone::getSpeed() * 5 / 60; // Speed * Time = Distance (in km) --- Not needed in this case

    // TSP algorithm takes an exponential amount of time to compute, so we either keep the number of cells small or pre-compute the paths
    // For simplicity, we will keep the number of cells small for each region (e.g., 10x10 grid)
    // Moving from the center of a cell to the center of another cell is equivalent to moving from the top-left corner of a cell to the top-left corner of another cell
    double droneCoverageArea = Drone::getVisibilityRange() * 2;
    droneCoverageArea *= droneCoverageArea; // Area = Side * Side
    return 10 * 10 * droneCoverageArea; // 10x10 grid of cells
}