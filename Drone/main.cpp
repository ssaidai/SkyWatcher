#include <iostream>
#include <thread>
#include <chrono>
#include "Drone/Drone.h"
#include "Redis/Redis.h"
#include "Utils/utils.h"

int main() {
    // Initialize a drone
    Drone drone;

    // Initialize redis
    RedisCommunication redisCommunication("127.0.0.1", 6379);
    DroneClient droneClient(redisCommunication.get_redis_instance());

    // Initialize the following threads
    std::thread batteryThread(&Drone::batteryThread, std::ref(drone));
    std::thread statusThread(&Drone::status, std::ref(drone));
    std::thread commandThread([&]() {
        droneClient.listen_for_commands([&](const std::string& cmd) {
            drone.executeCMD(cmd);
        });
    });



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