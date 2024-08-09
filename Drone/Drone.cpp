#include "random"
#include "thread"
#include "chrono"

#include "Drone.h"
#include "../Utils/utils.h"


// Initialize static members
const double Drone::flightAutonomy = 30;
const double Drone::rechargeTimeMin = 2;
const double Drone::rechargeTimeMax = 3;
const double Drone::speed = 30;
const double Drone::visibilityRange = 10;

// Constructor
Drone::Drone() {
    batteryLevel = 100; // Initialize battery level at maximum
}

// Assign a new path
void Drone::assignPath(Position destPoint) {
    if (this->state == DroneState::Ready) {
        // Initialize all Path params
        Position startPoint = {this->position.x, this->position.y};
        double distance = utils::calculateDistance(startPoint, destPoint);
        double travelTime = utils::calculateTime(distance, this->speed);
        double criticalBatteryLevel = getCriticalBatteryLevel(travelTime, this->flightAutonomy);

        // Assign a new path to the drone
        Path newPath = {startPoint, destPoint, distance, criticalBatteryLevel, travelTime};
        this->currentPath = newPath;

        // Drone should depart
        this->state = DroneState::Arriving;

    } else {
        // TODO: Error message
    }
}

// Simulate drone movement
void Drone::move() {
    if (this->state == DroneState::Arriving || this->state == DroneState::Returning) {
            // Update drone position: using linear interpolation: currentX = startX + ratio * deltaX
            double deltaX = std::abs(currentPath.startPoint.x - currentPath.destPoint.x);
            double deltaY = std::abs(currentPath.startPoint.y - currentPath.destPoint.y);

            // Ratio is the proportion between the distance currently traveled and the total distance
            double operationTime = Drone::getCurrentOperationTime();
            double ratio = (operationTime * this->speed / 3.6) / this->currentPath.distance;
            this->position.x = this->currentPath.startPoint.x + (ratio * deltaX);
            this->position.y = this->currentPath.startPoint.y + (ratio * deltaY);

    } else if (this->state == DroneState::Monitoring) {
        // TODO: Do it
    }
}

// Simulate drone battery consumption
void Drone::consumption() {
    // double consumptionRate = 100.0 / (this->flightAutonomy * 60.0);

    // TODO: Understand on which level should be implemented
}

// Simulate drone recharge
void Drone::recharge() {
    // Initializing random number generator based on current time and a uniform distribution between 2h and 3h in seconds
    std::mt19937 rng(static_cast<unsigned int> (std::time(nullptr)));
    std::uniform_real_distribution<double> chargeTimeDistribution(this->rechargeTimeMin * 3600.0, this->rechargeTimeMax * 3600.0);

    // chargeTime is a rng number in chargeTimeDistribution
    // chargeRate is the amount of battery charged every second
    double chargeTime = chargeTimeDistribution(rng);
    double chargeRate = (100.0 - this->batteryLevel) / chargeTime;

    // Charge until full capacity
    while (this->batteryLevel < 100) {
        // Charge for chargeRate amount every second
        this->batteryLevel += chargeRate;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Drone is fully charged and ready to operate
    this->state = DroneState::Ready;
}

double Drone::getCurrentOperationTime() const {
    double batteryUsed;
    switch (this->state) {
        case DroneState::Arriving:
            // Percentage of battery used in proportion to the flight autonomy in seconds
            return ( batteryUsed/ 100.0) * this->flightAutonomy * 60.0;

        case DroneState::Monitoring:
            // Percentage of battery used in the current cycle in proportion to the flight autonomy in seconds
            // TODO: Maybe save 14.0 in the Path struct (one monitoring round = 4m = 240s = 13.333% battery)
            batteryUsed = 14.0 - std::fmod((this->batteryLevel - (this->travelTime / this->flightAutonomy * 100.0)), 14.0);
            return (batteryUsed / 100.0) * this->flightAutonomy * 60.0;

        case DroneState::Returning:
            // Percentage of battery used in proportion to the flight autonomy in seconds
            batteryUsed = ((this->travelTime / this->flightAutonomy * 100.0) + std::fmod(100 - (this->travelTime / this->flightAutonomy * 100.0), 14)) - this->batteryLevel;
            return (batteryUsed / 100.0) * this->flightAutonomy * 60.0;

            // TODO: Maybe add case Recharging

        default:
            return -1.0;
    }
}

// Get the percentage of battery when the next drone should be called
double Drone::getCriticalBatteryLevel() const {
    return (2 * (currentPath.travelTime / flightAutonomy * 100.0))
}

// Check if drone battery is critical
bool Drone::isBatteryCritical() const {
    // Battery is critical when the next drone should be called
    return batteryLevel <= currentPath.criticalBatteryLevel;
}

// Check if drone battery is low
bool Drone::isBatteryLow() const {
    // Battery is low when the drone can't afford one more cycle
    return batteryLevel < ((currentPath.criticalBatteryLevel / 2.0) + 14.0);
}

// Get current drone position
Position Drone::getPosition() const {
    return position
}

// Get current battery level
double Drone::getBatteryLevel() const {
    return batteryLevel;
}

// Get current drone state
DroneState Drone::getDroneState() const {
    return state;
}