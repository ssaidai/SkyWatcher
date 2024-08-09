#include "utils.h"
#include "cmath"

// Calculate the distance between two points
double utils::calculateDistance (Position startPoint, Position destPoint) {
    return std::sqrt(std::pow(destPoint.x - startPoint.x, 2) + std::pow(destPoint.y - startPoint.y, 2));
}

// Calculate the time needed to travel given the distance and the speed
double utils::calculateTime (double distance, double speed) {
    return distance * (speed / 3.6)
}



