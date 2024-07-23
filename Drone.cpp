#include "Drone.h"


// Initialize static members
const double Drone::maxBatteryLevel = 100;
const double Drone::flightAutonomy = 30;
const double Drone::rechargeTimeMin = 2;
const double Drone::rechargeTimeMax = 3;
const double Drone::speed = 30;
const double Drone::visibilityRange = 10;

// Constructor
Drone::Drone() {
    batteryLevel = maxBatteryLevel; // Initialize battery level to maximum
}

// Simulate drone movement
void Drone::move(double distance) {
    double timeRequired = distance / speed; // Calculate time required to cover distance
    // Decrease battery level based on flight time
    batteryLevel -= (timeRequired / 60.0) * 100.0;
}

// Simulate drone recharge
void Drone::recharge() {
    // Randomly select recharge time within specified range
    double rechargeTime = rechargeTimeMin + static_cast<double>(rand()) / RAND_MAX * (rechargeTimeMax - rechargeTimeMin); // TODO: Use C++11 random library in the final implementation
    // Update battery level after recharge
    batteryLevel = std::min(batteryLevel + (rechargeTime / 3.0) * 100.0, maxBatteryLevel);
}

// Check if drone needs recharge
bool Drone::isBatteryLow() const {
    return batteryLevel < (flightAutonomy / 2.0); // Drone needs recharge if battery level is below 50%
}

// Get current battery level
double Drone::getBatteryLevel() const {
    return batteryLevel;
}

Position Drone::getPosition() const {
    return position;
}
