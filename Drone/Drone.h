#ifndef SKYWATCHER_DRONE_H
#define SKYWATCHER_DRONE_H

#include <memory>
#include "../Structs.h"
#include "../Redis/Redis.h"


class Drone {
private:
    Position position;                              // Current position
    std::mutex positionMutex;                       // Mutex for position
    Position towerPosition;                         // Tower position
    DroneState::Enum state;                         // Current drone state
    DroneClient redisClient;                        // Redis client

    int ID;                                 // Drone's ID (assigned once connected to the tower)
    double batteryLevel;                    // Current battery level
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
    void consumption();                                                           // Battery consumption
    void recharge();                                                             // Recharge battery
    void changeState(DroneState::Enum newState);                                // Change Drone's state
    void changeConsumptionRatio(double ratio);                                 // Change Drone's consumptionRate

    void move(Position dest, float travelTime);                               // Move toward dest
    void receiveDestination(Position destPoint, int sleepTime,               // Receive new destination
        std::array<Position, 100> waypoints, bool init);
    void moveToPosition(const Position& destination, float totalTravelTime);

    // Threads
    void batteryUpdateThread();                                             // Update drone's battery on redis
    [[noreturn]] void statusUpdateThread();                                // Update drone's status on redis

    [[nodiscard]] Position getPosition() const;
    [[nodiscard]] Position getDestination() const;
    [[nodiscard]] DroneState::Enum getDroneState() const;

    [[nodiscard]] int getCycleIteration(int sleepTime);
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