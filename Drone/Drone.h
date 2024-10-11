#ifndef SKYWATCHER_DRONE_H
#define SKYWATCHER_DRONE_H

#include <random>
#include <memory>
#include "../Structs.h"


class Drone {
private:
    Position position = {3000.0, 3000.0};      // Current position
    DroneState::Enum state;                          // Current drone state
    std::unique_ptr<Path> currentPath;              // Current path

    const int ID = 1;                       // Drone's unique ID TODO: Should be auto-increment
    double batteryLevel;                    // Current battery level
    double criticalBatteryLevel;            // When the next drone should be called
    double  consumptionRatio;               // Drone's battery consumption rate
    static const double consumptionRate;    // batteryConsumption/s
    static const double speed;              // Speed in m/s
    static const double flightAutonomy;     // Flight autonomy in minutes
    static const double rechargeTimeMin;    // Minimum recharge time in hours
    static const double rechargeTimeMax;    // Maximum recharge time in hours
    static const double visibilityRange;    // Visibility range in meters

public:
    Drone();    // Drone constructor
    ~Drone();   // Drone destructor

    // Drone function
    void consumption();                                       // Battery consumption
    void recharge();                                          // Recharge battery
    void changeState(DroneState::Enum state);                 // Change Drone' state
    void changeConsumptionRatio(double ratio);                // Change Drone's consumptionRate
    void receiveDestination(Position destPoint);              // Receive new destination
    void arrive();                                            // Move to the destination
    void monitor();                                           // Monitor the assigned sector
    void back();                                              // Move back

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