#ifndef SKYWATCHER_STRUCTS_H
#define SKYWATCHER_STRUCTS_H

struct Position {
    double x, y;
};

struct Path{
    double distance; // Distance
    Position start; // Current path's starting point
    Position destination; // Current path's destination point
    float timeNeeded; // Time needed to reach destination of the current path
    float timeTraveled;  // Time passed
};

enum DroneState {
   Charging,
   Ready,
   Flying,
   Waiting,
   Offline
};

#endif //SKYWATCHER_STRUCTS_H
