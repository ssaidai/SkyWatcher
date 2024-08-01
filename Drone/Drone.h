#ifndef SKYWATCHER_DRONE_H
#define SKYWATCHER_DRONE_H

#include <random>
#include "../Structs.h"


class Drone {
private:
    Position position = {3000, 3000, 0}; // Current position
    DroneState state; // Current drone state
    Path currentPath; // Current path

    double batteryLevel; // Current battery level
    static const double flightAutonomy; // Flight autonomy in minutes
    static const double rechargeTimeMin; // Minimum recharge time in hours
    static const double rechargeTimeMax; // Maximum recharge time in hours
    static const double speed; // Speed in km/h
    static const double visibilityRange; // Visibility range in meters

public:
    Drone();
    ~Drone(); // TODO: To be implemented
    void move(double x, double y);
    void recharge();
    void updatePosition();
    [[nodiscard]] Position getPosition() const;
    [[nodiscard]] DroneState getDroneState() const;
    [[nodiscard]] bool isBatteryLow() const;
    [[nodiscard]] double getBatteryLevel() const;

    // Static getters
    static double getFlightAutonomy() { return flightAutonomy; }
    static double getRechargeTimeMin() { return rechargeTimeMin; }
    static double getRechargeTimeMax() { return rechargeTimeMax; }
    static double getSpeed() { return speed; }
    static double getVisibilityRange() { return visibilityRange; }
};
#endif //SKYWATCHER_DRONE_H