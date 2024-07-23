#ifndef SKYWATCHER_GRIDDEFINITIONS_H
#define SKYWATCHER_GRIDDEFINITIONS_H

#include <vector>
#include "Structs.h"

class Cell {
public:
    float left, right, top, bottom;
    Position center;

    // Ensure there's a default constructor
    Cell() : left(0), right(0), top(0), bottom(0), center({0, 0, 0}) {}

    // Custom constructor
    Cell(float l, float r, float t, float b) : left(l), right(r), top(t), bottom(b), center({(left + right) / 2, (top + bottom) / 2, 0}) {}
};


// A sector is a 10x10 sub-grid of cells
class Sector {
private:
    int sectorID;
    std::vector<std::vector<Cell>> grid;

public:
    Sector(int sectorID, int startX, int startY, std::vector<std::vector<Cell>>& allCells) {
        this->sectorID = sectorID;
        grid.resize(10, std::vector<Cell>(10));
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                grid[i][j] = allCells[startY + i][startX + j];
            }
        }
    }

    // Optionally provide methods to access or manipulate cells within the sector
    [[nodiscard]] const std::vector<std::vector<Cell>>& getGrid() const {
        return grid;
    }
};

#endif //SKYWATCHER_GRIDDEFINITIONS_H
