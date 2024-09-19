#ifndef SKYWATCHER_DRONE_H
#define SKYWATCHER_DRONE_H

#include <random>
#include "../Structs.h"


class Drone {
private:
    Position position = {3000, 3000}; // Current position
    DroneState::Enum state; // Current drone state
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
    void assignPath(Position destPoint);
    void move();
    void recharge();
    void consumption();
    [[nodiscard]] Position getPosition() const;
    [[nodiscard]] DroneState getDroneState() const;
    [[nodiscard]] bool isBatteryCritical() const;
    [[nodiscard]] bool isBatteryLow() const;
    [[nodiscard]] double getBatteryLevel() const;

    [[nodiscard]] double getCurrentOperationTime () const;
    [[nodiscard]] double getCriticalBatteryLevel () const;

    // Static getters
    static double getFlightAutonomy() { return flightAutonomy; }
    static double getRechargeTimeMin() { return rechargeTimeMin; }
    static double getRechargeTimeMax() { return rechargeTimeMax; }
    static double getSpeed() { return speed; }
    static double getVisibilityRange() { return visibilityRange; }
};
#endif //SKYWATCHER_DRONE_H