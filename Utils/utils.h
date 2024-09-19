#ifndef SKYWATCHER_UTILS_H
#define SKYWATCHER_UTILS_H

#include "Drone.h"
#include "Structs.h"

namespace utils {
    double calculateDistance (Position startPoint, Position destPoint); // Calculate the distance between two points in metres
    double calculateTime (double distance, double speed); // Calculate the time needed to travel in seconds
}


#endif //SKYWATCHER_UTILS_H
