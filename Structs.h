#ifndef SKYWATCHER_STRUCTS_H
#define SKYWATCHER_STRUCTS_H


struct Position {
    double x;
    double y;
};

struct Path {
    Position startPoint; // Starting point
    Position destPoint; // Destination point

    static const double distance; // Distance
    static const double criticalBatteryLevel; // When the next drone should be called
    static const float travelTime; // Time needed to reach destination of the current path
};

struct DroneState {
    enum Enum {
        Ready, // Drone is ready to operate
        Charging, // Charging battery
        Waiting, // Waiting all drone to start together
        Arriving, // Going to the destination
        Monitoring, // Monitoring a sector
        Returning, // Returning to the tower
        Offline // Drone is not operative
    };
};

#endif //SKYWATCHER_STRUCTS_H
