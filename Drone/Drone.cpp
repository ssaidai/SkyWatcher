#include "Drone.h"


// Initialize static members
const double Drone::flightAutonomy = 30;
const double Drone::rechargeTimeMin = 2;
const double Drone::rechargeTimeMax = 3;
const double Drone::speed = 30;
const double Drone::visibilityRange = 10;

// Constructor
Drone::Drone() {
    batteryLevel = 100; // Initialize battery level to maximum
}

// Simulate drone movement
void Drone::move(double x, y) { // Use Position struct

}

// Update drone position
void Drone::updatePosition() {
    // Using linear interpolation: currentX = startX + ratio * deltaX

    // Use this
    // Path path = this->currentPath;
    // Position delta {std::abs(path.start.x - path.destination.x), std::abs(path.start.y - path.destination.y)}
    double deltaX = std::abs(currentPath.startX - currentPath.destX);
    double deltaY = std::abs(currentPath.startY - currentPath.destY);
    
    // Ratio is the proportion between the distance currently traveled and the total distance
    double ratio = (currentPath.timeTraveled * speed) / currentPath.distance; // TODO: Controllare l'unità di misura di timeTraveled

    // Use . for structs and -> for classes
    this.position.x = currentPath.startX + (ratio * deltaX);
    this.position.y = currentPath.startY + (ratio * deltaY);
}

// Simulate drone recharge, loop this in the battery thread
void Drone::recharge() {
    // Randomly select recharge time within specified range
    double rechargeTime = rechargeTimeMin + static_cast<double>(rand()) / RAND_MAX * (rechargeTimeMax - rechargeTimeMin); // TODO: Use C++11 random library in the final implementation

    // sleep thread for recharge time

    // Update battery level after recharge
    batteryLevel = std::min(batteryLevel + (rechargeTime / 3.0) * 100.0, maxBatteryLevel);
}

// Get current drone position
Position Drone::getPosition() const {
    return position
}

// Check if drone needs recharge
bool Drone::isBatteryLow() const {
    return batteryLevel < (flightAutonomy / 2.0); // Drone needs recharge if battery level is below 50%
}

// Get current battery level
double Drone::getBatteryLevel() const {
    return batteryLevel;
}

// Get current drone state
DroneState Drone::getDroneState() const {
    return state;
}