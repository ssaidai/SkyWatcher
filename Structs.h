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
