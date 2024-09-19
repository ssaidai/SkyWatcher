#include "WatchZone.h"

WatchZone::WatchZone() {
    // Initialize sectors
    // Get sector number based on region area and cell size
    float cellSize = 20; // Assuming 20m x 20m cells
    size_t cellsPerSector = 10; // 10x10 cells per sector
    auto sectorSize = static_cast<size_t>(cellsPerSector * cellSize);

    [[maybe_unused]] auto numSectorsPerRow = static_cast<std::size_t>(this->width / sectorSize); // Calculate sectors per row, maybe not used

    auto numRows = static_cast<std::size_t>(std::ceil(this->height / cellSize));
    auto numCols = static_cast<std::size_t>(std::ceil(this->width / cellSize));

    // Initialize the whole grid first (could be done lazily if memory is a concern)
    std::vector<std::vector<Cell>> allCells(numRows, std::vector<Cell>(numCols));

    // Initialize cell boundaries (optional, depending on Cell constructor)
    for (int i = 0; i < height / cellSize; i++) {
        for (int j = 0; j < width / cellSize; j++) {
            allCells[i][j] = Cell(j * cellSize, (j + 1) * cellSize, i * cellSize, (i + 1) * cellSize);
        }
    }

    // Create sectors
    int sectorID = 0;
    for (size_t y = 0; y < height; y += sectorSize) {
        for (size_t x = 0; x < width; x += cellsPerSector * cellSize) {
            sectors.push_back(std::make_unique<Sector>(sectorID++, x / cellSize, y / cellSize, allCells));
        }
    }

    // Initialize drones
    for (int i = 0; i < droneCount; i++) {
        drones.emplace_back(); // Changed from drones.push_back(Drone()); for efficiency
    }

    // Initialize cerebrum and pass in the sectors
    this->cerebrum = Cerebrum();
    cerebrum.solveTSP(sectors);

    // Initialize Redis connection
    const std::string host = "127.0.0.1"; // TODO: Take this from config file or make it as an input variable
    const int port = 6379;
    RedisCommunication redisCommunication(host, port);
    TowerClient redis(redisCommunication.get_redis_instance());
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