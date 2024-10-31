#ifndef SKYWATCHER_GRIDDEFINITIONS_H
#define SKYWATCHER_GRIDDEFINITIONS_H

#include <vector>
#include "Utils/utils.h"

class Cell {
private:
    float left, right, top, bottom;
    Position center;

public:
    // Ensure there's a default constructor
    Cell() : left(0), right(0), top(0), bottom(0), center({0, 0}) {}

    // Custom constructor
    Cell(float l, float r, float t, float b) : left(l), right(r), top(t), bottom(b), center({(left + right) / 2, (top + bottom) / 2}) {}

    [[nodiscard]] Position getCenter() const {
        return center;
    }
};


// A sector is a 10x10 sub-grid of cells
class Sector {
private:
    int sectorID, assignedDroneID, regionID;
    float areaSize;
    std::vector<std::vector<Cell*>> grid;
    Position startingPoint{};
    std::array<Position, 100> waypoints{};
    std::array<Position, 100> path{};
    double distance;
    int timer;
    int starting_index;

public:
    Sector(int sectorID, int startX, int startY, const std::vector<std::vector<std::shared_ptr<Cell>>>& allCells, const int size) : assignedDroneID(-1), areaSize(size) {
        this->sectorID = sectorID;
        this->grid.resize(10, std::vector<Cell*>(10));
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                this->grid[i][j] = allCells[startY + i][startX + j].get();
                waypoints[i * 10 + j] = this->grid[i][j]->getCenter();
            }
        }
        // Set the starting point based on the sector's position (starting point should be the center of the closest cell to the center of the area)
        if(const float temp = (areaSize / 10) / 4; startY < temp && startX < temp){
            // Top-left region
            startingPoint = this->grid[9][9]->getCenter();
            regionID = 0;
            starting_index = 99;
        }
        else if(startY < temp){
            // Top-right region
            startingPoint = this->grid[9][0]->getCenter();
            regionID = 1;
            starting_index = 90;
        }
        else if(startX < temp){
            // Bottom-left region
            startingPoint = this->grid[0][9]->getCenter();
            regionID = 2;
            starting_index = 9;
        }
        else{
            // Bottom-right region
            startingPoint = this->grid[0][0]->getCenter();
            regionID = 3;
            starting_index = 0;
        }

        // Calculate travelTime
        distance = utils::calculateDistance(Position{areaSize / 2,areaSize / 2}, startingPoint);
        const int travelTime = static_cast<int>(utils::calculateTime(distance, 30 / 3.6));

        const int temp = 1800 - 2 * travelTime;

        // Calculate the time after which the tower should send a new drone to this sector
        timer = temp - static_cast<int>(std::fmod(temp, 240));
    }

    void assignDrone(int droneID) {
        this->assignedDroneID = droneID;
    }

    [[nodiscard]] std::array<Position, 100> getTSP() const
    {
        return path;
    }

    [[nodiscard]] int getTimer() const {
        return timer;
    }

    [[nodiscard]] int getStartingIndex() const {
        return starting_index;
    }

    [[nodiscard]] const std::array<Position, 100>& getWaypoints() const {
        return waypoints;
    }

    [[nodiscard]] int getRegionID() const {
        return regionID;
    }

    void setTSP(const std::array<Position, 100>& path) {
        Position offset = this->startingPoint;
        std::transform(path.begin(), path.end(), this->path.begin(),
                   [offset](const Position& pos) {
                       return pos + offset;
                   });
    }

    [[nodiscard]] int getSectorID() const {
        return this->sectorID;
    }

    [[nodiscard]] int getAssignedDroneID() const {
        return this->assignedDroneID;
    }

    [[nodiscard]] Position getStartingPoint() const {
        return this->startingPoint;
    }

    // Optionally provide methods to access or manipulate cells within the sector
    [[nodiscard]] const std::vector<std::vector<Cell*>>& getGrid() const {
        return grid;
    }
};

#endif //SKYWATCHER_GRIDDEFINITIONS_H
