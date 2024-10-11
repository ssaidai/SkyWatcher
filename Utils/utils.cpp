#include "utils.h"
#include "cmath"
#include <string>
#include "Drone/Drone.h"

// Calculate the distance between two points
double utils::calculateDistance (Position startPoint, Position destPoint) {
    return std::sqrt(std::pow(destPoint.x - startPoint.x, 2) + std::pow(destPoint.y - startPoint.y, 2));
}

// Calculate the time needed to travel given the distance and the speed
float utils::calculateTime (double distance, double speed) {
    return static_cast<float>(distance * speed);
}

// Get the percentage of battery when the next drone should be called
double utils::getCriticalBatteryLevel(double travelTime, const double flightAutonomy) {
    double deviation = calculateDeviation(travelTime, flightAutonomy);
    return (2 * (travelTime / flightAutonomy * 100.0)) + deviation;
}

// Calculate the deviation from critical battery level
double utils::calculateDeviation(double travelTime, double flightAutonomy) {
    return std::fmod(100.0 - (2 * (travelTime / flightAutonomy * 100.0)), 14.0);
}

std::string utils::statusToJSON(Drone& drone) {
    std::string droneState = "State: " + std::to_string(drone.getDroneState());
    std::string position = "Position: (" + std::to_string(drone.getPosition().x) + ", "
            + std::to_string(drone.getPosition().y) + ")";
    std::string batteryLevel = "Battery Level: " + std::to_string(drone.getBatteryLevel()) + "%";
    std::string critical = "Critical Point: " + std::to_string(drone.isBatteryCritical());
    return "{" + droneState + ", " + position + ", " + batteryLevel + ", " + critical + "}";
}
