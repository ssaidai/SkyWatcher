#ifndef SKYWATCHER_STRUCTS_H
#define SKYWATCHER_STRUCTS_H
#include <nlohmann/json.hpp>

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

    Position operator+(const Position& other) const {
        return {x + other.x, y + other.y};
    }

    Position operator-(const Position& other) const {
        return {x - other.x, y - other.y};
    }

    Position operator~() const {
        return {std::floor((x * 100) / 100), std::floor((y * 100) / 100)};
    }
};

// Define how to serialize Position to JSON
inline void to_json(nlohmann::json& j, const Position& pos) {
    j = nlohmann::json{{"x", pos.x}, {"y", pos.y}};
}

// Define how to deserialize JSON to Position
inline void from_json(const nlohmann::json& j, Position& pos) {
    j.at("x").get_to(pos.x);
    j.at("y").get_to(pos.y);
}

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

    [[nodiscard]] static std::string toString (const Enum state) {
        switch (state) {
            case Ready: return "Ready";
            case Charging: return "Charging";
            case Waiting: return "Waiting";
            case Arriving: return "Arriving";
            case Monitoring: return "Monitoring";
            case Returning: return "Returning";
            case Offline: return "Offline";
        }
        return "Unknown";
    }
};

struct Status {
    DroneState::Enum state;
    Position position;

    double batteryLevel;
};

#endif //SKYWATCHER_STRUCTS_H
