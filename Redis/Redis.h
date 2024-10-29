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

    std::shared_ptr<Redis> get_redis_instance() {
        return redis;
    }

private:
    std::shared_ptr<Redis> redis;
};

// Tower Client (for controlling drones)
class TowerClient {
public:
    explicit TowerClient(const std::shared_ptr<Redis> &redis, std::vector<std::shared_ptr<Sector>> &s, const int timeScale, const Position pos) : redis(redis), drone_id_counter(0), sectors(s), timeScale(timeScale), tower_position(pos){}

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

    void start_substitution_listener()
    {
        std::thread substitution_thread([this]() {
            this->listen_for_substitution_msg();
        });
        substitution_thread.detach();
    }

    // Optionally broadcast a command to all drones
    void broadcast_command(const std::string &command) const
    {
        redis->publish("drone:broadcast", command);  // Publish to a broadcast channel
        std::cout << "Broadcast command: " << command << std::endl;
    }

    std::unordered_map<int, nlohmann::json> get_drone_statuses() {
        std::lock_guard lock(drones_mutex);
        return drone_statuses; // Returns a copy of the map
    }

private:
    std::shared_ptr<Redis> redis;
    std::vector<std::shared_ptr<Sector>> sectors;
    std::unordered_map<int, std::shared_ptr<Sector>> drone_to_sector_map;
    std::mutex sectors_mutex;
    std::atomic<int> drone_id_counter;
    int timeScale;

    Position tower_position;
    std::mutex drones_mutex;
    std::unordered_set<int> active_drones;  // Track active drones
    std::unordered_set<int> waiting_drones;  // Track drones waiting for a sector
    std::unordered_map<int, nlohmann::json> drone_statuses;  // Store drone statuses
    std::unordered_map<int, std::chrono::system_clock::time_point> drone_initialization_time;


    // Listen for new drone connections on the handshake channel
    void listen_for_drone_connections() {
        auto subscriber = redis->subscriber();
        subscriber.subscribe("drone:handshake");

        // Handle incoming handshake messages from drones
        subscriber.on_message([this](const std::string&, const std::string& message) {
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

    void listen_for_substitution_msg()
    {
        auto subscriber = redis->subscriber();
        subscriber.subscribe("drone:go_next");

        subscriber.on_message([this](const std::string&, const std::string& message)
        {
           nlohmann::json msg = nlohmann::json::parse(message);
           substituteDrone(msg["drone_id"]);
        });

        try {
            while (true) {
                subscriber.consume();
            }
        } catch (const Error &err) {
            std::cerr << "Error consuming substitution messages: " << err.what() << std::endl;
        }
    }

    void substituteDrone(const int droneID)
    {
        std::cout << "Substitution message received: " << droneID << std::endl;
        std::lock_guard lock(sectors_mutex);
        std::shared_ptr<Sector> sector = drone_to_sector_map[droneID];

        {
            std::lock_guard lock(drones_mutex);
            active_drones.erase(droneID);
            drone_to_sector_map.erase(droneID);
            waiting_drones.insert(droneID);
        }

        for(const auto newDroneID : waiting_drones)
        {
            if(auto status = drone_statuses[newDroneID]; status["state"] == "Ready")
            {
                sector->assignDrone(newDroneID);
                drone_to_sector_map[newDroneID] = sector;
                {
                    std::lock_guard lock(drones_mutex);
                    waiting_drones.erase(newDroneID);
                    active_drones.insert(newDroneID);
                }

                const std::string channel = "drone:" + std::to_string(newDroneID) + ":commands";
                const nlohmann::json msg = {{"starting_point", sector->getStartingPoint()}, {"timer", sector->getTimer()}, {"tsp", sector->getTSP()}};
                redis->publish(channel, msg.dump());
                std::cout << "Drone " << droneID << " substituted with " << newDroneID <<std::endl;
                break;
            }
        }
    }

    void monitor_drones() {
        while (true) {
            std::vector<int> drones_to_check;
            {
                std::lock_guard lock(drones_mutex);
                drones_to_check.assign(active_drones.begin(), active_drones.end());
                drones_to_check.insert(drones_to_check.end(), waiting_drones.begin(), waiting_drones.end());
            }

            int counter = 0;
            for (int drone_id : drones_to_check) {
                // Check if the drone is within the grace period
                if (auto init_time_it = drone_initialization_time.find(drone_id); init_time_it != drone_initialization_time.end()) {
                    auto now = std::chrono::system_clock::now();
                    if (auto duration_since_init = std::chrono::duration_cast<std::chrono::seconds>(now - init_time_it->second); duration_since_init.count() < 5) { // Grace period in seconds
                        // Skip checking this drone until grace period is over
                        continue;
                    }
                }

                std::string status_key = "drone:" + std::to_string(drone_id) + ":status";

                try {
                    if (auto status_opt = redis->get(status_key)) {
                        // Drone is alive; process status if needed
                        {
                            const nlohmann::json status = nlohmann::json::parse(*status_opt);
                            if(status["state"] == "Waiting")
                                counter++;
                            std::lock_guard lock(drones_mutex);
                            drone_statuses[drone_id] = status;
                        }
                    } else {
                        // Drone may be unresponsive
                        handle_unresponsive_drone(drone_id);
                    }
                } catch (const Error &err) {
                    std::cerr << "Error fetching status for drone " << drone_id << ": " << err.what() << std::endl;
                }
            }
            if(counter && counter == active_drones.size())
            {
                broadcast_command("START");
            }

            std::this_thread::sleep_for(std::chrono::duration<float>(0.1 / timeScale)); // Adjust as needed
        }
    }

    void handle_unresponsive_drone(const int drone_id) {
        std::cout << "Drone " << drone_id << " is not responding. Taking action!" << std::endl;

        {
            std::lock_guard lock(sectors_mutex);
            // Remove the drone from its sector
            if (const auto it = drone_to_sector_map.find(drone_id); it != drone_to_sector_map.end()) {
                it->second->assignDrone(-1);
                drone_to_sector_map.erase(it);
            }
        }

        {
            std::lock_guard lock(drones_mutex);
            // Remove the drone from active drones
            active_drones.erase(drone_id);
            // Remove its status
            drone_statuses.erase(drone_id);
        }
        // Additional actions can be taken, such as alerting operators or reassigning tasks
    }

    // Initialize the drone by assigning it a unique ID and sending initialization data
    void initialize_drone(const std::string &drone_info_json) {
        // Parse the received drone "hello" message
        auto drone_info = nlohmann::json::parse(drone_info_json);
        const std::string drone_uuid = drone_info["drone_uuid"];
        std::lock_guard lock(sectors_mutex);

        // Assign a unique drone ID
        int new_drone_id = ++drone_id_counter;

        // Create an initialization message with the assigned ID and an area to monitor
        nlohmann::json init_message = {
                {"drone_id", new_drone_id},
                {"tower_position", tower_position}
        };

        {
            std::lock_guard lock2(drones_mutex);
            waiting_drones.insert(new_drone_id);
            drone_initialization_time[new_drone_id] = std::chrono::system_clock::now();
        }

        // Assign the drone to a sector
        for (const auto &sector : sectors) {
            if (sector->getAssignedDroneID() == -1){
                sector->assignDrone(new_drone_id);
                drone_to_sector_map[new_drone_id] = sector;
                {
                    std::lock_guard lock2(drones_mutex);
                    waiting_drones.erase(new_drone_id);
                    active_drones.insert(new_drone_id);
                }
                Position startingPoint = sector->getStartingPoint();
                init_message = {
                        {"drone_id", new_drone_id},
                        {"tower_position", tower_position},
                        {"starting_point", startingPoint},
                        {"timer", sector->getTimer()},
                        {"tsp", sector->getTSP()}
                };
                break;
            }
        }

        // Send initialization message back to the drone
        const std::string drone_channel = "drone:" + drone_uuid + ":init";
        redis->publish(drone_channel, init_message.dump());

        std::cout << "Drone " << drone_uuid << " initialized with ID: " << new_drone_id << std::endl;
    }
};

// Drone Client (for receiving commands and sending status updates)
class DroneClient {
public:
     DroneClient(const std::shared_ptr<Redis> &redis, int timeScale)
            : redis(redis), drone_uuid(generate_uuid()), timeScale(timeScale) {}

    // Send a handshake to the tower to register the drone
    void connect_to_tower(const std::function<void(const nlohmann::json &)>& callback) {
        // Listen for initialization message from the tower
        std::thread init_listener_thread([this, callback]() {
            this->listen_for_initialization(callback);
        });
         init_listener_thread.detach();
         std::this_thread::sleep_for(std::chrono::duration<float>(0.05 / timeScale));  // Wait for the listener to start

        nlohmann::json handshake_message = {
                {"drone_uuid", drone_uuid}
        };

        // Publish a handshake message to the tower
        redis->publish("drone:handshake", handshake_message.dump());
    }

    // Start sleeping thread
    void start_sleeping_thread(const nlohmann::json& message, float seconds) const
    {
        std::thread sleeping_thread([this, message, seconds]() {
            std::this_thread::sleep_for(std::chrono::duration<float>(seconds / timeScale));
            redis->publish("drone:go_next", message.dump());
        });
        sleeping_thread.detach();
    }

    std::shared_ptr<Redis> getRedisInstance() {
         return redis;
     }

    // Start listening for commands after initialization
    void listen_for_commands(const std::function<void(const nlohmann::json &)> &callback) const
    {
        const std::string drone_command_key = "drone:" + std::to_string(drone_id) + ":commands";
         auto subscriber = redis->subscriber();
        subscriber.subscribe(drone_command_key);
         bool flag = true;

         subscriber.on_message([&subscriber, &flag, callback, this](const std::string&, const std::string& message)
         {
             callback(nlohmann::json::parse(message));
             subscriber.unsubscribe("drone:" + std::to_string(drone_id) + ":commands");
             flag = false;
         });

         try
         {
             while (flag)
                 subscriber.consume();
         } catch (const Error &err)
         {
             std::cerr << "Error consuming command messages: " << err.what() << std::endl;
         }
    }

    void listen_for_broadcasts(const std::function<void(const std::string &)> &callback) const
    {
        auto subscriber = redis->subscriber();
        subscriber.subscribe("drone:broadcast");
         bool flag = true;

        subscriber.on_message([&subscriber, &flag , callback](const std::string&, const std::string& message) {
            callback(message);
            subscriber.unsubscribe("drone:broadcast");
            flag = false;
        });

        try {
            while (flag)
                subscriber.consume();
        } catch (const Error &err) {
            std::cerr << "Error consuming broadcast messages: " << err.what() << std::endl;
        }
    }

    // Send status update to the tower
    void send_status_update(const nlohmann::json &status) const
    {
        const std::string status_key = "drone:" + std::to_string(drone_id) + ":status";
        redis->set(status_key, status.dump(), std::chrono::seconds(3));  // Update status in Redis with a TTL of 3 seconds
        std::cout << "Drone " << drone_id << " status updated: " << status << std::endl;

         if (status["state"] == "Monitoring") {
             // Also append the status to a central Redis Stream
             std::string stream_key = "status_logs";
             nlohmann::json status_log = status;
             status_log["timestamp"] = getCurrentTime();  // Include a timestamp

             // Prepare fields for xadd
             std::vector<std::pair<std::string, std::string>> fields = {{"status", status_log.dump()}};

             // Add the status update to the central stream
             redis->xadd(stream_key, "*", fields.begin(), fields.end());

         }
    }

private:
    std::shared_ptr<Redis> redis;
    std::string drone_uuid;     // Unique drone identifier
    int drone_id;               // Assigned after initialization
    int timeScale;

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

        bool flag = true;

        subscriber.on_message([this, callback, &subscriber, &flag](const std::string& channel, const std::string& message) {
            // Parse the initialization message
            auto init_message = nlohmann::json::parse(message);
            drone_id = init_message["drone_id"];

            std::cout << "Drone initialized with ID: " << drone_id << std::endl;

            if (callback) {
                callback(init_message);
                subscriber.unsubscribe(channel);
                flag = false;
            }
        });

        // Wait for initialization message
        try {
            while(flag)
                subscriber.consume();
        } catch (const Error &err) {
            std::cerr << "Error subscribing to initialization: " << err.what() << std::endl;
        }
    }
};

#endif  // REDIS_COMMUNICATION_HPP