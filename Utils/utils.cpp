#include "utils.h"
#include "cmath"
#include "Drone/Drone.h"

// Calculate the distance between two points
double utils::calculateDistance (Position startPoint, Position destPoint) {
    return std::sqrt(std::pow(destPoint.x - startPoint.x, 2) + std::pow(destPoint.y - startPoint.y, 2));
}

// Calculate the time needed to travel given the distance and the speed
float utils::calculateTime (double distance, double speed) {
    return static_cast<float>(distance / speed);
}
