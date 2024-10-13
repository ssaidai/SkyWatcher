#include <iostream>
#include <thread>
#include <chrono>
#include "Drone/Drone.h"
#include "Redis/Redis.h"
#include "Utils/utils.h"

// TODO: Implement the threads in the drone class, the main file should only generate a bunch of child processes

// Drone's battery thread implementation
void battery(Drone& drone) {
    while (drone.getDroneState() != DroneState::Offline) {

        if (drone.getDroneState() == DroneState::Charging) {
            drone.recharge(); // It stops the thread and wait until the drone is fully charged
        }
        else {
            // Battery consumption
            drone.consumption();

            // Verify if the drone should return
            if (drone.isBatteryLow() && (drone.getPosition() == drone.getDestination())) {
                drone.back();
            }
        }

        // Wait a secondo before the next check
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


void status(Drone& drone) {
    while (drone.getDroneState() != DroneState::Offline) {
        std::string status = utils::statusToJSON(drone);
        send_status_update(status);

        std::this_thread::sleep_for(std::chrono::seconds (1));
    }
}

void executeCMD(const std::string& command) {
    if (command == "xxx") {

    }
}

int main() {
    // Initialize a drone
    Drone drone;

    // Initialize redis
    RedisCommunication redisCommunication("127.0.0.1", 6379);
    DroneClient droneClient(redisCommunication.get_redis_instance(), drone.getID());

    // Initialize the following threads
    std::thread batteryThread(battery, std::ref(drone));
    std::thread statusThread(status, std::ref(drone));
    std::thread commandThread(&DroneClient::listen_for_commands, &droneClient, executeCMD);


    // Wait for battery's thread to end
    if (batteryThread.joinable()) {
        batteryThread.join();
    }

    // Wait for status's thread to end
    if (statusThread.joinable()) {
        statusThread.join();
    }

    // Wait for command's thread to end
    if (commandThread.joinable()) {
        commandThread.join();
    }

    return 0;
}
