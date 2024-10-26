#ifndef REDIS_COMMUNICATION_HPP
#define REDIS_COMMUNICATION_HPP

#include <sw/redis++/redis++.h>
#include <nlohmann/json.hpp>
#include <string>
#include <functional>
#include <iostream>
#include <atomic>
#include <thread>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "GridDefinitions.h"
#include "Utils/Logger.h"

using namespace sw::redis;

// Core Redis communication class (establishes a connection to Redis)
class RedisCommunication {
public:
    RedisCommunication(const std::string &host, const int port) {
        ConnectionOptions connection_options;
        connection_options.host = host;  // Redis host
        connection_options.port = port;  // Redis port

        // Create a connection to Redis
        redis = std::make_shared<Redis>(connection_options);
    }

    ~RedisCommunication() {
        // Proper cleanup of the Redis connection
        redis->flushall();  // Optional, clears all keys; can be removed if not needed
    }

    std::shared_ptr<Redis> get_redis_instance() {
        return redis;
    }

private:
    std::shared_ptr<Redis> redis;
};

// Tower Client (for controlling drones)
class TowerClient {
public:
    explicit TowerClient(const std::shared_ptr<Redis> &redis, std::vector<std::shared_ptr<Sector>> &s) : redis(redis), drone_id_counter(0), sectors(s){}

    // Start a listener thread to handle new drone connections
    void start_listening_for_drones() {
        std::thread listener_thread([this]() {
            this->listen_for_drone_connections();
        });
        listener_thread.detach();
    }

    void start_monitoring_drones() {
        std::thread monitor_thread([this]() {
            this->monitor_drones();
        });
        monitor_thread.detach();
    }

    // Send a command to a specific drone
    void send_command_to_drone(const std::string &drone_id, const std::string &command) {
        std::string drone_command_key = "drone:" + drone_id + ":commands";
        redis->rpush(drone_command_key, command);  // Use a list to enqueue commands for the drone
        std::cout << "Command sent to drone " << drone_id << ": " << command << std::endl;
    }

    // Optionally broadcast a command to all drones
    void broadcast_command(const std::string &command) {
        redis->publish("drone:broadcast", command);  // Publish to a broadcast channel
        std::cout << "Broadcast command: " << command << std::endl;
    }

private:
    std::shared_ptr<Redis> redis;
    std::vector<std::shared_ptr<Sector>> sectors;
    std::unordered_map<int, std::shared_ptr<Sector>> drone_to_sector_map;
    std::mutex sectors_mutex;
    std::atomic<int> drone_id_counter;

    // Listen for new drone connections on the handshake channel
    void listen_for_drone_connections() {
        auto subscriber = redis->subscriber();
        subscriber.subscribe("drone:handshake");

        // Handle incoming handshake messages from drones
        subscriber.on_message([this](const std::string& channel, const std::string& message) {
            this->initialize_drone(message);
        });

        // Could be improved with threading to handle multiple connections concurrently

        // Continuously consume handshake messages
        try {
            while (true) {
                subscriber.consume();
            }
        } catch (const Error &err) {
            std::cerr << "Error consuming handshake messages: " << err.what() << std::endl;
        }
    }

    bool isAlive(int droneID) const
    {
        std::string key = "drone:" + std::to_string(droneID) + ":status";
        auto exists = redis->exists(key);
        return exists == 1;
    }

    [[noreturn]] void monitor_drones(){
        while (true) {
            for (auto &entry : drone_to_sector_map) {
                if (!isAlive(entry.first)) {
                    std::cout << "Drone " << std::to_string(entry.first) << " is not responding. Taking action!" << std::endl;
                    // Here, you can trigger substitute logic or reassign tasks
                    std::lock_guard lock(sectors_mutex);
                    entry.second->assignDrone(-1);
                    drone_to_sector_map.erase(entry.first);
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(5));  // Check every 5 seconds
        }
    }

    // Initialize the drone by assigning it a unique ID and sending initialization data
    void initialize_drone(const std::string &drone_info_json) {
        // Parse the received drone "hello" message
        auto drone_info = nlohmann::json::parse(drone_info_json);
        std::string drone_uuid = drone_info["drone_uuid"];
        std::lock_guard lock(sectors_mutex);

        // Assign a unique drone ID
        int new_drone_id = ++drone_id_counter;

        // Create an initialization message with the assigned ID and an area to monitor
        nlohmann::json init_message = {
                {"drone_id", new_drone_id},
                {"tower_position", {
                    {"x", 3000},
                    {"y", 3000}}
                }
        };

        // Assign the drone to a sector
        for (const auto &sector : sectors) {
            if (sector->getAssignedDroneID() == -1){
                sector->assignDrone(new_drone_id);
                drone_to_sector_map[new_drone_id] = sector;
                Position startingPoint = sector->getStartingPoint();
                init_message = {
                        {"drone_id", new_drone_id},
                        {"tower_position", {
                            {"x", 3000},
                            {"y", 3000}}
                        },
                        {"starting_point", {
                            {"x", startingPoint.x},
                            {"y", startingPoint.y}}
                        },
                        {"timer", sector->getTimer()}
                };
                break;
            }
        }

        // Send initialization message back to the drone
        std::string drone_channel = "drone:" + drone_uuid + ":init";
        redis->publish(drone_channel, init_message.dump());

        std::cout << "Drone " << drone_uuid << " initialized with ID: " << new_drone_id << std::endl;
    }
};

// Drone Client (for receiving commands and sending status updates)
class DroneClient {
public:
     DroneClient(const std::shared_ptr<Redis> &redis)
            : redis(redis), drone_uuid(generate_uuid()) {}

    // Send a handshake to the tower to register the drone
    void connect_to_tower(const std::function<void(const nlohmann::json &)>& callback) {
        // Listen for initialization message from the tower
        std::thread init_listener_thread([this, callback]() {
            this->listen_for_initialization(callback);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Wait for the listener to start

        nlohmann::json handshake_message = {
                {"drone_uuid", drone_uuid}
        };

        // Publish a handshake message to the tower
        redis->publish("drone:handshake", handshake_message.dump());

        init_listener_thread.join();  // Wait for initialization to complete
    }

    // Start sleeping thread
    void start_sleeping_thread(const nlohmann::json& message, int seconds) const
    {
        std::thread sleeping_thread([this, message, seconds]() {
            std::this_thread::sleep_for(std::chrono::seconds(seconds));
            redis->publish("drone:go_next", message.dump());
        });
        sleeping_thread.detach();
    }

    // Start listening for commands after initialization
    [[noreturn]] void listen_for_commands(const std::function<void(const std::string &)> &callback) const
    {
        const std::string drone_command_key = "drone:" + std::to_string(drone_id) + ":commands";

        while (true) {
            if (const auto command = redis->blpop(drone_command_key, 0)) {
                std::string cmd = command->second;
                callback(cmd);  // Execute the callback with the received command
            }
        }
    }

    // Send status update to the tower
    void send_status_update(const nlohmann::json &status) const
    {
        std::string status_key = "drone:" + std::to_string(drone_id) + ":status";
        redis->set(status_key, status.dump(), std::chrono::seconds(3));  // Update status in Redis with a TTL of 3 seconds
        std::cout << "Drone " << drone_id << " status updated: " << status << std::endl;

        // Also append the status to a central Redis Stream
        std::string stream_key = "status_logs";
        nlohmann::json status_log = status;
        status_log["timestamp"] = getCurrentTime();  // Include a timestamp

        // Prepare fields for xadd
        std::vector<std::pair<std::string, std::string>> fields = {{"status", status_log.dump()}};

        // Add the status update to the central stream
        redis->xadd(stream_key, "*", fields.begin(), fields.end());
    }

private:
    std::shared_ptr<Redis> redis;
    std::string drone_uuid;     // Unique drone identifier
    int drone_id;               // Assigned after initialization

    // Generate a UUID for the drone name
    std::string generate_uuid() {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        return boost::uuids::to_string(uuid);
    }

    // Listen for initialization message from the tower
    void listen_for_initialization(const std::function<void(const nlohmann::json &)>& callback) {
        auto subscriber = redis->subscriber();
        std::string drone_channel = "drone:" + drone_uuid + ":init";
        subscriber.subscribe(drone_channel);

        subscriber.on_message([this, callback](const std::string& channel, const std::string& message) {
            // Parse the initialization message
            auto init_message = nlohmann::json::parse(message);
            drone_id = init_message["drone_id"];

            std::cout << "Drone initialized with ID: " << drone_id << std::endl;

            if (callback) {
                callback(init_message);
            }
        });

        // Wait for initialization message
        try {
            subscriber.consume();
        } catch (const Error &err) {
            std::cerr << "Error subscribing to initialization: " << err.what() << std::endl;
        }
    }
};

#endif  // REDIS_COMMUNICATION_HPP