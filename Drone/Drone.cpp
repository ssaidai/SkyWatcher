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
const double Drone::speed = 30.0 / 3.6;       // 30km/h in m/s
const double Drone::visibilityRange = 10;
const double Drone::consumptionRate = 100.0 / (flightAutonomy * 60.0);  // consumptionRate/second
constexpr float cellTravelTime = 20.0 / (30.0 / 3.6);


// Destructor
Drone::~Drone() = default; // Change implementation if new resources should be manually freed

// Constructor
Drone::Drone() : redisClient(RedisCommunication("127.0.0.1", 6379).get_redis_instance()) {
    this->batteryLevel = 100; // Initialize battery level at maximum
    this->state = DroneState::Ready;
    this->consumptionRatio = 1.0;

    // Initialize connection to tower
    redisClient.connect_to_tower([this](nlohmann::json init_message) {  // Lambda function to assign droneID, could be a member function
        // Init_message parse
        this->ID = init_message["drone_id"];
        this->towerPosition = {init_message["tower_position"][0], init_message["tower_position"][1]};
        this->position = this->towerPosition;
        Position startPoint = {init_message["starting_point"][0], init_message["starting_point"][1]};
        int sleepTime = init_message["timer"];

        std::array<Position, 100> tsp = init_message["tsp"]; // TODO: TO BE IMPLEMENTED

        // Initialize operation
        this->receiveDestination(startPoint, sleepTime, tsp, true);
    });

    // Start the status update thread
    std::thread statusUpdateThread(&Drone::statusUpdateThread, this);
    statusUpdateThread.detach();

    std::thread batteryUpdateThread(&Drone::batteryUpdateThread, this);
    batteryUpdateThread.detach();

    // Run this in the listener thread
    redisClient.listen_for_commands([this](const std::string &command) {
        if (command == "recallAll") {
            this->changeState(DroneState::Returning);
        }
    });

}



// Receive new path from the tower
// init=true if it's called from the constructor, else init=false
void Drone::receiveDestination(Position startPoint, int sleepTime,
    std::array<Position, 100> waypoints, bool init=false) {
    if (this->state == DroneState::Ready) {
        // Initialize Path's params
        double distance = utils::calculateDistance(this->position, startPoint);
        float travelTime = utils::calculateTime(distance, Drone::speed);


        // Drone Arriving
        this->changeState(DroneState::Arriving);
        this->move(startPoint, travelTime);


        // Drone Waiting
        if (init) {
            this->changeState(DroneState::Waiting);
            this->changeConsumptionRatio(0.5);
            // TODO: listen for broadcast
            this->changeConsumptionRatio(1.0);
        }


        // Drone monitoring
        this->changeState(DroneState::Monitoring);
        int cycleIteration = this->getCycleIteration(sleepTime);
        for(int i = 0; i < cycleIteration; i++) {
            // The drone is going through every cell waypoint following the TSP-CPP algorithm
            for (auto waypoint: waypoints) {
                // Wait for the drone reaching the assigned sector
                std::this_thread::sleep_for(std::chrono::duration<float>(cellTravelTime));
                this->position = waypoint;
            }
        }

        // Drone Returning
        this->changeState(DroneState::Returning);
        this->move(this->towerPosition, travelTime);

        // Drone Charging
        this->changeState(DroneState::Charging);
        this->recharge();

        // TODO: disconnect from reddis
    }
}

// Simulate drone's movement toward an assigned destination
void Drone::move(Position dest, float travelTime) {
    // Wait for the drone reaching the destination
    std::this_thread::sleep_for(std::chrono::duration<double>(travelTime));
    this->position = dest;

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

// Drone's battery thread implementation
void Drone::batteryUpdateThread() {
    while (this->getDroneState() != DroneState::Offline && this->getDroneState() != DroneState::Charging) {
        // Battery consumption
        this->consumption();

        // Wait a secondo before the next consumption
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int Drone::getCycleIteration(int sleepTime) {
    int cycleTime = 240; // TODO: 240?
    return sleepTime / cycleTime;
}

// Get current drone position
Position Drone::getPosition() const {
    return this->position;
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


