#ifndef SKYWATCHER_STRUCTS_H
#define SKYWATCHER_STRUCTS_H
#include <array>

#include <array>

struct Position {
    double x;
    double y;

    // Override == operation
    bool operator==(const Position& other) const {
        return (x == other.x) && (y == other.y);
    }

    // Override != operation
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

struct Path {
    Position startPoint; // Starting point

    std::array<Position, 100> waypoints; // Cell visit order according to the TCP-CPP
    const double distance; // Distance
    const float travelTime; // Time needed to reach destination of the current path


    // Constructor
    Path(const Position& start, const std::array<Position, 100>& wp, double dist, float time)
            : startPoint(start), waypoints(wp), distance(dist), travelTime(time) {}
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

struct Status {
    DroneState::Enum state;
    Position position;

    double batteryLevel;
};

#endif //SKYWATCHER_STRUCTS_H
