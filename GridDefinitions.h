#ifndef SKYWATCHER_GRIDDEFINITIONS_H
#define SKYWATCHER_GRIDDEFINITIONS_H

#include <vector>
#include "Structs.h"

class Cell {
private:
    float left, right, top, bottom;
    Position center;

public:
    // Ensure there's a default constructor
    Cell() : left(0), right(0), top(0), bottom(0), center({0, 0}) {}

    // Custom constructor
    Cell(float l, float r, float t, float b) : left(l), right(r), top(t), bottom(b), center({(left + right) / 2, (top + bottom) / 2}) {}

    Position getCenter() const {
        return center;
    }
};


// A sector is a 10x10 sub-grid of cells
class Sector {
private:
    int sectorID;
    std::vector<std::vector<Cell*>> grid;
    Position startingPoint;

public:
    Sector(int sectorID, int startX, int startY, std::vector<std::vector<std::shared_ptr<Cell>>>& allCells) {
        this->sectorID = sectorID;
        this->grid.resize(10, std::vector<Cell*>(10));
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                this->grid[i][j] = allCells[startY + i][startX + j].get();
            }
        }
        // Set the starting point based on the sector's position (starting point should be the center of the closest cell to the center of the area)
        if(startY < 15 && startX < 15){
            // Top-left region
            startingPoint = this->grid[9][9]->getCenter();
        }
        else if(startY < 15){
            // Top-right region
            startingPoint = this->grid[9][0]->getCenter();
        }
        else if(startX < 15){
            // Bottom-left region
            startingPoint = this->grid[0][9]->getCenter();
        }
        else{
            // Bottom-right region
            startingPoint = this->grid[0][0]->getCenter();
        }
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
