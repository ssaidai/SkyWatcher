#ifndef SKYWATCHER_UTILS_H
#define SKYWATCHER_UTILS_H

#include "Structs.h"

namespace utils {
    double calculateDistance (Position startPoint, Position destPoint); // Calculate the distance between two points in metres
    float calculateTime (double distance, double speed); // Calculate the time needed to travel in seconds
}

#endif //SKYWATCHER_UTILS_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


