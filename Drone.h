#ifndef SKYWATCHER_DRONE_H
#define SKYWATCHER_DRONE_H

#include "random"

struct Position {
    double x,y,z;
};

class Drone {
private:
    Position position = {0, 0, 0}; // Current position
    double batteryLevel; // Current battery level

    static const double maxBatteryLevel; // Maximum battery level
    static const double flightAutonomy; // Flight autonomy in minutes
    static const double rechargeTimeMin; // Minimum recharge time in hours
    static const double rechargeTimeMax; // Maximum recharge time in hours
    static const double speed; // Speed in km/h
    static const double visibilityRange; // Visibility range in meters

public:
    Drone();
    void move(double distance);
    void recharge();
    [[nodiscard]] Position getPosition() const;
    [[nodiscard]] bool isBatteryLow() const;
    [[nodiscard]] double getBatteryLevel() const;

    // Static getters
    static double getMaxBatteryLevel() { return maxBatteryLevel; }
    static double getFlightAutonomy() { return flightAutonomy; }
    static double getRechargeTimeMin() { return rechargeTimeMin; }
    static double getRechargeTimeMax() { return rechargeTimeMax; }
    static double getSpeed() { return speed; }
    static double getVisibilityRange() { return visibilityRange; }
};

// Initialize static members
const double Drone::maxBatteryLevel = 100;
const double Drone::flightAutonomy = 30;
const double Drone::rechargeTimeMin = 2;
const double Drone::rechargeTimeMax = 3;
const double Drone::speed = 30;
const double Drone::visibilityRange = 10;



#endif //SKYWATCHER_DRONE_H
