#ifndef SKYWATCHER_DRONE_H
#define SKYWATCHER_DRONE_H

#include <random>
#include <memory>
#include "../Structs.h"


class Drone {
private:
    Position position = {3000.0, 3000.0}; // Current position
    DroneState::Enum state; // Current drone state
    std::unique_ptr<Path> currentPath; // Current path

    const int ID = 1; // Drone's unique ID TODO: Should be auto-increment
    double batteryLevel; // Current battery level
    double criticalBatteryLevel; // When the next drone should be called
    static const double flightAutonomy; // Flight autonomy in minutes
    static const double rechargeTimeMin; // Minimum recharge time in hours
    static const double rechargeTimeMax; // Maximum recharge time in hours
    static const double speed; // Speed in m/s
    static const double visibilityRange; // Visibility range in meters
    static const double consumptionRate; // batteryConsumption/s

public:
    Drone();
    ~Drone(); // TODO: To be implemented
    void receiveDestination(Position destPoint);
    void arrive();
    void monitor();
    void back();
    void recharge();
    void consumption(double rate);
    void changeState(DroneState::Enum state);
    [[nodiscard]] Position getPosition() const;
    [[nodiscard]] Position getDestination() const;
    [[nodiscard]] DroneState::Enum getDroneState() const;
    [[nodiscard]] bool isBatteryCritical() const;
    [[nodiscard]] bool isBatteryLow() const;
    [[nodiscard]] double getBatteryLevel() const;
    [[nodiscard]] int getID() const;

    // Static getters
    static double getFlightAutonomy() { return flightAutonomy; }
    static double getRechargeTimeMin() { return rechargeTimeMin; }
    static double getRechargeTimeMax() { return rechargeTimeMax; }
    static double getSpeed() { return speed; }
    static double getVisibilityRange() { return visibilityRange; }
};
#endif //SKYWATCHER_DRONE_H