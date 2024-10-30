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
const double Drone::consumptionRate = 100.0 / (flightAutonomy * 60.0);  // consumptionRate/second
const double Drone::visibilityRange = 10;
constexpr float cellTravelTime = 20.0 / (30.0 / 3.6);


// Constructor
Drone::Drone(int timeScale) : redisClient(RedisCommunication("127.0.0.1", 6379).get_redis_instance(), timeScale), timeScale(timeScale) {
    this->batteryLevel = 100.0; // Initialize battery level at maximum
    this->state = DroneState::Ready;
    this->consumptionRatio = 1.0;
    //this->consumptionRate = 100.0 / (flightAutonomy * 60.0);  // consumptionRate/second

    // Initialize connection to tower
    redisClient.connect_to_tower([this](nlohmann::json init_message) {  // Lambda function to assign droneID, could be a member function
        // Init_message parse
        this->ID = init_message["drone_id"];
        this->towerPosition = init_message["tower_position"];
        this->position = this->towerPosition;

        // Start the status update thread
        std::thread statusUpdateThread(&Drone::statusUpdateThread, this);
        statusUpdateThread.detach();

        std::thread batteryUpdateThread(&Drone::batteryUpdateThread, this);
        batteryUpdateThread.detach();

        if(init_message.contains("timer")) {
            const int sleepTime = init_message["timer"];
            const Position startPoint = init_message["starting_point"];
            const std::array<Position, 100> tsp = init_message["tsp"];

            // Initialize operation
            this->receiveDestination(startPoint, sleepTime, tsp, true);
        }
        else {
            wait_for_path();
        }
    });

    std::cin.get();
}

void Drone::wait_for_path() {
    redisClient.listen_for_commands([this](const nlohmann::json& command)
    {
        const Position startPoint = command["starting_point"];
        const int sleepTime = command["timer"];
        const std::array<Position, 100> tsp = command["tsp"];

        this->receiveDestination(startPoint, sleepTime, tsp, false);
    });
}

// Receive new path from the tower
// init=true if it's called from the constructor, else init=false
void Drone::receiveDestination(Position startPoint, int sleepTime,
                               std::array<Position, 100> waypoints, bool init = false) {
    if (init) {
        std::this_thread::sleep_for(std::chrono::seconds(6));
    }
    if (this->state == DroneState::Ready) {
        // Initialize Path's parameters
        const double distance = utils::calculateDistance(this->position, startPoint);
        const float travelTime = utils::calculateTime(distance, speed);


        // Drone Arriving
        this->changeState(DroneState::Arriving);
        this->moveToPosition(startPoint, travelTime);


        // Drone Waiting
        if (init) {
            this->changeState(DroneState::Waiting);
            this->changeConsumptionRatio(0.0);
            // Implement any waiting logic here
            redisClient.listen_for_broadcasts([this](const std::string& message) {
                std::cout << "Received broadcast: " << message << std::endl;
            });
            this->changeConsumptionRatio(1.0);
        }


        // Thead for subsequent drone call
        nlohmann::json message = {{"drone_id", this->ID}};
        redisClient.start_sleeping_thread(message, sleepTime-travelTime);
        // Drone Monitoring
        this->changeState(DroneState::Monitoring);
        int cycleIteration = this->getCycleIteration(sleepTime);
        double wp_distance = utils::calculateDistance(waypoints[0], waypoints[1]);
        float wp_travelTime = utils::calculateTime(wp_distance, speed);
        for (int i = 0; i < cycleIteration; i++) {
            for (const auto& waypoint : waypoints) {
                this->moveToPosition(waypoint, wp_travelTime);
            }
        }


        // Drone Returning
        this->changeState(DroneState::Returning);
        this->moveToPosition(this->towerPosition, travelTime);


        // Drone Charging
        this->changeState(DroneState::Charging);
        this->recharge();
    }
}

void Drone::moveToPosition(const Position& destination, float totalTravelTime) {

    totalTravelTime /= timeScale;
    // Record the start time
    auto startTime = std::chrono::steady_clock::now();
    Position startPosition{};
    {
        std::lock_guard lock(positionMutex);
        startPosition = this->position;
    }

    float elapsedTime = 0.0f;


    Position newPosition{};
    while (elapsedTime < totalTravelTime && this->state != DroneState::Offline) {
        // Calculate the fraction of travel completed
        float fraction = elapsedTime / totalTravelTime;

        // Interpolate the position
        newPosition.x = startPosition.x + fraction * (destination.x - startPosition.x);
        newPosition.y = startPosition.y + fraction * (destination.y - startPosition.y);

        // Update the drone's position
        {
            std::lock_guard lock(positionMutex);
            this->position = newPosition;
        }

        // Sleep for the update interval
        std::this_thread::sleep_for(std::chrono::microseconds(1));

        // Update the elapsed time
        auto currentTime = std::chrono::steady_clock::now();
        elapsedTime = std::chrono::duration<float>(currentTime - startTime).count();
    }

    // Ensure the final position is set exactly to the destination
    {
        std::lock_guard lock(positionMutex);
        this->position = destination;
    }
}

// Simulate drone battery consumption
void Drone::consumption() {
    // Battery level should never be fall below 0%
    std::lock_guard lock(batteryMutex);
    {
        this->batteryLevel = std::max(this->batteryLevel - consumptionRate * this->consumptionRatio, 0.0);

        // Out of charge check
        if (this->batteryLevel == 0.0) {
            this->changeState(DroneState::Offline);
        }
    }
}

// Simulate drone recharge
void Drone::recharge() {
    double rechargeTime = rechargeTimeMin + static_cast<double>(rand()) / RAND_MAX * (rechargeTimeMax - rechargeTimeMin);
    double rechargeRate = (100.0 - this->batteryLevel) / (rechargeTime * 3600.0);

    while (this->batteryLevel < 100.0) {
        double rechargeAmount = rechargeRate * timeScale;
        this->batteryLevel = std::min(this->batteryLevel + rechargeAmount, 100.0);
        std::this_thread::sleep_for(std::chrono::duration<float>(1.0f));
    }

    this->state = DroneState::Ready;
    wait_for_path();
}

// Change drone state
void Drone::changeState(const DroneState::Enum newState) {
    this->state = newState;
}

void Drone::changeConsumptionRatio(double ratio) {
    this->consumptionRatio = ratio;
}

void Drone::statusUpdateThread() {
    // Send status update to the tower
    std::this_thread::sleep_for(std::chrono::duration<float>(0.5));
    while (this->state != DroneState::Offline) {
        // Create a json object with the drone status
        nlohmann::json status = {
            {"drone_id", this->ID},
            {"position", ~this->position},
            {"battery_level", std::floor(this->batteryLevel * 100.0) / 100.0},
            {"state", DroneState::toString(this->state)},
            {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
        };

        // Send the status update
        this->redisClient.send_status_update(status);

        // Wait for 3 seconds before the next update
        std::this_thread::sleep_for(std::chrono::duration<float>(1.0 / timeScale));
    }
}

// Drone's battery thread implementation
void Drone::batteryUpdateThread() {
    while (this->state != DroneState::Offline) {
        // Battery consumption
        if (this->state != DroneState::Charging && this->state != DroneState::Ready) {
            this->consumption();
        }
        // Wait a secondo before the next consumption
        std::this_thread::sleep_for(std::chrono::duration<float>(1.0 / timeScale));
    }
}

int Drone::getCycleIteration(const int sleepTime) {
    constexpr int cycleTime = 240;
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


