// WatchZone.cpp
#include "WatchZone.h"

std::vector<std::shared_ptr<Sector>> WatchZone::createSectors() {
    float cellSize = 20; // Assuming 20m x 20m cells
    size_t cellsPerSector = 10; // 10x10 cells per sector
    auto sectorSize = static_cast<size_t>(cellsPerSector * cellSize);

    auto numRows = static_cast<std::size_t>(std::ceil(this->height / cellSize));
    auto numCols = static_cast<std::size_t>(std::ceil(this->width / cellSize));

    std::vector allCells(numRows, std::vector<std::shared_ptr<Cell>>(numCols)); // Vector not needed use std::array instead

    for (int i = 0; i < height / cellSize; i++) {
        for (int j = 0; j < width / cellSize; j++) {
            allCells[i][j] = std::make_shared<Cell>(j * cellSize, (j + 1) * cellSize, i * cellSize, (i + 1) * cellSize);
        }
    }

    std::vector<std::shared_ptr<Sector>> sectors;
    sectors.reserve(9000);  // Change this to be dynamic
    int sectorID = 0;
    for (size_t y = 0; y < height; y += sectorSize) {
        for (size_t x = 0; x < width; x += cellsPerSector * cellSize) {
            int startX = static_cast<int>(x / cellSize);
            int startY = static_cast<int>(y / cellSize);
            sectors.emplace_back(std::make_shared<Sector>(sectorID++, startX, startY, allCells));
        }
    }

    return sectors;
}

WatchZone::WatchZone()
        : sectors(createSectors()), // Initialize sectors using the new method
          cerebrum(sectors), // Initialize cerebrum with the newly created sectors
          redisCommunication("127.0.0.1", 6379),
          client(redisCommunication.get_redis_instance(), sectors)
{
    // Listen for drone connections
    client.start_listening_for_drones();
    std::cout << "Listening for drone connections..." << std::endl;

    // Look for disconnected drones
    client.start_monitoring_drones();
    std::cout << "Monitoring drones..." << std::endl;

    // Send a command to drone with ID "drone_1"
    //// redis.send_command_to_drone("drone_1", "Move to area (10, 20)");

    // Broadcast a command to all drones
    //// redis.broadcast_command("All drones perform maintenance check.");

    // Initialize the TSP solver with the paths to all sectors
}



// wtf?? need to implement redis communication for this
Position WatchZone::getDronePosition(int droneID) {
    // query redis to get the position

    return drones[droneID].getPosition();
}

// This function calculates the area size of the region that a single drone can cover in 5 minutes
[[maybe_unused]] double getRegionArea() {
    // Calculate the distance that a drone can cover in 5 minutes
    // double distance = Drone::getSpeed() * 5 / 60; // Speed * Time = Distance (in km) --- Not needed in this case

    // TSP algorithm takes an exponential amount of time to compute, so we either keep the number of cells small or pre-compute the paths
    // For simplicity, we will keep the number of cells small for each region (e.g., 10x10 grid)
    // Moving from the center of a cell to the center of another cell is equivalent to moving from the top-left corner of a cell to the top-left corner of another cell
    double droneCoverageArea = Drone::getVisibilityRange() * 2;
    droneCoverageArea *= droneCoverageArea; // Area = Side * Side
    return 10 * 10 * droneCoverageArea; // 10x10 grid of cells
}