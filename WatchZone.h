#ifndef SKYWATCHER_WATCHZONE_H
#define SKYWATCHER_WATCHZONE_H

#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include "Drone.h"

class Cell {
public:
    float left, right, top, bottom;
    Position center;

    // Ensure there's a default constructor
    Cell() : left(0), right(0), top(0), bottom(0), center(0, 0, 0) {}

    // Custom constructor
    Cell(float l, float r, float t, float b) : left(l), right(r), top(t), bottom(b), center((left + right) / 2, (top + bottom) / 2, 0) {}
};


// A sector is a 10x10 grid of cells
class Sector {
private:
    std::vector<std::vector<Cell>> grid;

public:
    Sector(int cellSize, int startX, int startY, std::vector<std::vector<Cell>>& allCells) {
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


// WatchZone class using grid implementation and TSP algorithm
class WatchZone {
private:
    float width, height = 6000;
    Position center = {3000, 3000, 0};
    std::vector<std::unique_ptr<Sector>> sectors;

public:
    WatchZone();
};


#endif //SKYWATCHER_WATCHZONE_H
