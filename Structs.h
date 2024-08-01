#ifndef SKYWATCHER_STRUCTS_H
#define SKYWATCHER_STRUCTS_H

struct Position {
    double x, y, z;
};

struct Path{
    double distance; // Distance
    double startX, startY; // Current path's starting point
    double destX, destY; // Current path's destination point
    float timeNeeded; // Time needed to reach destination of the current path
    float startedBy;  // Time passed
};

struct DroneState {
    enum Enum {
        Recharging,
        Waiting,
        Running,
        Ready
    };
};

#endif //SKYWATCHER_STRUCTS_H
