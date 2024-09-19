#ifndef SKYWATCHER_REDIS_H
#define SKYWATCHER_REDIS_H

// To use this you need hiredis and redis-plus-plus libraries
#include <sw/redis++/redis++.h>
#include <string>
#include <functional>
#include <iostream>

// TODO: change all standard outputs to use a logging lib

using namespace sw::redis;

class RedisCommunication {
public:
    RedisCommunication(const std::string &host, int port) {
        ConnectionOptions connection_options;
        connection_options.host = host;  // Redis host
        connection_options.port = port;  // Redis port

        // Create a connection to Redis
        redis = std::make_shared<Redis>(connection_options);
    }

    ~RedisCommunication() {
        // Proper cleanup of the redis connection
        redis->flushall();
    }

    std::shared_ptr<Redis> get_redis_instance() {
        return redis;
    }

private:
    std::shared_ptr<Redis> redis;
};

// Tower Client (for sending commands to drones)
class TowerClient {
public:
    TowerClient(const std::shared_ptr<Redis> &redis) : redis(redis) {}

    // Send command to a specific drone
    void send_command_to_drone(const std::string &drone_id, const std::string &command) {
        std::string drone_command_key = "drone:" + drone_id + ":commands";
        redis->rpush(drone_command_key, command);  // Use a list to enqueue commands for the drone
        std::cout << "Command sent to drone " << drone_id << ": " << command << std::endl;
    }

    // Optionally, send a broadcast command to all drones (for maintenance or global update)
    void broadcast_command(const std::string &command) {
        redis->publish("drone:broadcast", command);  // Publish command to a broadcast channel
        std::cout << "Broadcast command: " << command << std::endl;
    }

private:
    std::shared_ptr<Redis> redis;
};

// Drone Client (for receiving commands and sending status updates)
class DroneClient {
public:
    DroneClient(const std::shared_ptr<Redis> &redis, const std::string &drone_id)
            : redis(redis), drone_id(drone_id) {}

    // Listen to commands sent to this drone
    void listen_for_commands(const std::function<void(const std::string &)> &callback) {
        std::string drone_command_key = "drone:" + drone_id + ":commands";

        while (true) {
            auto command = redis->blpop(drone_command_key, 0);  // Blocking pop (wait for a command)
            if (command) {
                std::string cmd = command->second;
                callback(cmd);  // Execute the callback with the received command
            }
        }
    }

    // Send status update (like position or monitoring data) back to the tower
    void send_status_update(const std::string &status) {
        std::string status_key = "drone:" + drone_id + ":status";
        redis->set(status_key, status);  // Use a simple key-value pair for storing status
        std::cout << "Drone " << drone_id << " status updated: " << status << std::endl;
    }

private:
    std::shared_ptr<Redis> redis;
    std::string drone_id;
};

#endif //SKYWATCHER_REDIS_H
