#include "random"
#include "thread"
#include "chrono"
#include "cmath"

#include "Drone.h"
#include "Utils/utils.h"


// Initialize static members
const double Drone::flightAutonomy = 30;
const double Drone::rechargeTimeMin = 2;
const double Drone::rechargeTimeMax = 3;
const double Drone::speed = 30 / 3.6;       // 30km/h in m/s
const double Drone::visibilityRange = 10;
const double Drone::consumptionRate = 100.0 / (Drone::flightAutonomy * 60.0); // consumptionRate/second

// Constructor
Drone::Drone() : redisClient(RedisCommunication("127.0.0.1", 6379).get_redis_instance()) {
    this->batteryLevel = 100; // Initialize battery level at maximum
    this->state = DroneState::Ready;
    this->criticalBatteryLevel = 0.0;
    this->consumptionRatio = 1.0;

    // Initialize connection to tower
    redisClient.connect_to_tower([this](int droneID) {  // Lambda function to assign droneID, could be a member function
        this->ID = droneID;
    });

    // Start the status update thread
    std::thread statusUpdateThread(&Drone::statusUpdateThread, this);
    statusUpdateThread.detach();

    // Run this in the listener thread
    redisClient.listen_for_commands([this](const std::string &json) {
        // Execute the command after parsing json message
        //this->executeCMD(command);
    });

}

// Destructor
Drone::~Drone() = default; // Change implementation if new resources should be manually freed

[[noreturn]] void Drone::statusUpdateThread() {
    // Send status update to the tower
    while (true) {
        // Create a json object with the drone status
        nlohmann::json status = {
                {"drone_id", this->ID},
                {"position", {this->position.x, this->position.y}},
                {"battery_level", this->batteryLevel},
                {"state", this->state}
        };

        // Send the status update
        this->redisClient.send_status_update(status);

        // Wait for 3 seconds before the next update
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Receive new path from station
void Drone::receiveDestination(Position destPoint) {
    if (this->state == DroneState::Ready) {
        // Initialize Path's params
        Position startPoint = {this->position.x, this->position.y};
        double distance = utils::calculateDistance(startPoint, destPoint);
        float travelTime = utils::calculateTime(distance, Drone::speed);
        std::array<Position, 100> foo{}; // TODO: TO BE IMPLEMENTED

        // Assign a new path to the drone
        this->currentPath = std::make_unique<Path>(startPoint, destPoint, foo, distance,  travelTime);

        // Assign new critical battery level
        this->criticalBatteryLevel = utils::getCriticalBatteryLevel(travelTime, Drone::flightAutonomy);

        // Drone should depart
        this->state = DroneState::Arriving;
    }
}

// Simulate drone's movement toward the assigned sector
void Drone::arrive() {
    // Wait for the drone reaching the assigned sector
    std::this_thread::sleep_for(std::chrono::duration<double>(this->currentPath->travelTime));
    this->position = this->currentPath->destPoint;

    // Change drone's state
    this->changeState(DroneState::Waiting);
    // TODO: this->changeConsumptionRatio();
}

// Simulate drone movement
void Drone::monitor() {
    // The drone is going through every cell waypoint following the TSP-CPP algorithm
    for (auto waypoint: this->currentPath->waypoints) {
        // We need the travel time between the current cell and the next one TODO: travel time should be constant
        double cellDistance = utils::calculateDistance(this->position, waypoint);
        float cellTravelTime = utils::calculateTime(cellDistance, Drone::speed);


        // Wait for the drone reaching the assigned sector
        std::this_thread::sleep_for(std::chrono::duration<double>(cellTravelTime));
        this->position = waypoint;
    }
}

// Simulate drone return
void Drone::back(){
    // Wait for the drone to return
    this->changeState(DroneState::Returning);
    std::this_thread::sleep_for(std::chrono::duration<double>(this->currentPath->travelTime));
    this->position = this->currentPath->startPoint;

    // Change drone's state
    this->changeState(DroneState::Charging);
}

// Simulate drone battery consumption
void Drone::consumption() {
    // Battery level should never be fall below 0%
    this->batteryLevel = std::max(this->batteryLevel - Drone::consumptionRate * this->consumptionRatio, 0.0);

    // Out of charge check
    if (this->batteryLevel == 0.0) {
        this->changeState(DroneState::Offline);
    }
}

// Simulate drone recharge
void Drone::recharge() {
    /* TODO: To be used in the final version
    // Initializing random number generator based on current time and a uniform distribution between 2h and 3h in seconds
    std::mt19937 rng(static_cast<unsigned int> (std::time(nullptr)));
    std::uniform_real_distribution<double> chargeTimeDistribution(Drone::rechargeTimeMin * 3600.0, Drone::rechargeTimeMax * 3600.0);

    // chargeTime is a rng number in chargeTimeDistribution
    // chargeRate is the amount of battery charged every second
    double rechargeTime = chargeTimeDistribution(rng);
    double rechargeRate = (100.0 - this->batteryLevel) / rechargeTime;

    // Charge until full capacity
    while (this->batteryLevel < 100) {
        // Charge for rechargeRate amount every second
        this->batteryLevel += rechargeRate;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    */

    // Randomly select recharge time within specified range
    double rechargeTime = Drone::rechargeTimeMin + static_cast<double>(rand()) / RAND_MAX * (Drone::rechargeTimeMax - Drone::rechargeTimeMin);
    // rechargeRate for every second
    double rechargeRate = (100.0 - this->batteryLevel) / (rechargeTime * 3600.0);

    // Charge until full capacity
    while (this->batteryLevel < 100) {
        // Battery level should never exceed 100%
        this->batteryLevel = std::max(this->batteryLevel + rechargeRate, 100.0);

        // Wait a secondo before the next iteration
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Drone is fully charged and ready to operate
    this->state = DroneState::Ready;
}

// Change drone state
void Drone::changeState(DroneState::Enum newState) {
    this->state = newState;
}

void Drone::changeConsumptionRatio(double ratio) {
    this->consumptionRatio = ratio;
}

// Check if drone battery is critical
bool Drone::isBatteryCritical() const {
    // Battery is critical when the subsequent drone should be called (1% tolerance)
    return this->batteryLevel <= this->criticalBatteryLevel + 1;
}

// Check if drone battery is low
bool Drone::isBatteryLow() const {
    // Battery is low when the drone can't afford one more cycle and should return (1% tolerance)
    return this->batteryLevel <= this->criticalBatteryLevel + utils::calculateDeviation(this->currentPath->travelTime,
                                                                                        Drone::flightAutonomy) + 1;
}

// Get current drone position
Position Drone::getPosition() const {
    return this->position;
}

// Get current drone destination
Position Drone::getDestination() const {
    return this->currentPath->destPoint;
}

// Get current battery level
double Drone::getBatteryLevel() const {
    return this->batteryLevel;
}

// Get current drone state
DroneState::Enum Drone::getDroneState() const {
    return this->state;
}

int Drone::getID() const {
    return this->ID;
}


