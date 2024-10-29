#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include <algorithm>
#include <iomanip>    // For std::get_time
#include <sstream>    // For std::istringstream
#include "../Redis/Redis.h"


using namespace sw::redis;

// Function to retrieve status logs from Redis
void retrieve_status_logs_from_redis(std::shared_ptr<Redis> redis,
                                     std::vector<nlohmann::json> &status_logs,
                                     std::chrono::system_clock::time_point &simulation_start_time,
                                     std::chrono::system_clock::time_point &simulation_end_time) {
    std::string stream_key = "status_logs";

    // Fetch all entries from the stream
    // Prepare the vector to hold entries
    std::vector<std::pair<std::string, std::unordered_map<std::string, std::string>>> entries;

    // Fetch all entries from the stream
    redis->xrange(stream_key, "-", "+", std::back_inserter(entries));


    if (entries.empty()) {
        std::cerr << "No entries found in the status_logs stream." << std::endl;
        return;
    }

    simulation_start_time = std::chrono::system_clock::time_point::min();
    simulation_end_time = std::chrono::system_clock::time_point::min();

    for (const auto &entry : entries) {
        auto status_iter = entry.second.find("status");
        if (status_iter != entry.second.end()) {
            nlohmann::json status = nlohmann::json::parse(status_iter->second);

            // Parse timestamp
            std::string timestamp_str = status["timestamp"];
            std::tm tm = {};
            std::istringstream ss(timestamp_str);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            if (ss.fail()) {
                std::cerr << "Failed to parse timestamp: " << timestamp_str << std::endl;
                continue;
            }
            std::time_t time_t_timestamp = std::mktime(&tm);
            std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::from_time_t(time_t_timestamp);

            // Update simulation start and end times
            if (simulation_start_time == std::chrono::system_clock::time_point::min() || timestamp < simulation_start_time) {
                simulation_start_time = timestamp;
            }
            if (simulation_end_time == std::chrono::system_clock::time_point::min() || timestamp > simulation_end_time) {
                simulation_end_time = timestamp;
            }

            // Add status to status_logs
            status_logs.push_back(status);
        }
    }
}

// Helper function to format time points
std::string format_time_point(const std::chrono::system_clock::time_point &time_point) {
    std::time_t time_t_timestamp = std::chrono::system_clock::to_time_t(time_point);
    std::tm tm = *std::localtime(&time_t_timestamp);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return buffer;
}

// Function to parse status logs
void parse_status_logs(const std::vector<nlohmann::json> &status_logs,
                       std::map<std::pair<int, int>, std::vector<std::chrono::system_clock::time_point>> &cell_visit_times) {
    for (const auto &status : status_logs) {
        int drone_id = status["drone_id"];
        double x = status["position"]["x"];
        double y = status["position"]["y"];
        double battery_level = status["battery_level"];
        std::string timestamp_str = status["timestamp"];

        // Parse timestamp
        std::tm tm = {};
        std::istringstream ss(timestamp_str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if (ss.fail()) {
            std::cerr << "Failed to parse timestamp: " << timestamp_str << std::endl;
            continue;
        }
        std::time_t time_t_timestamp = std::mktime(&tm);
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::from_time_t(time_t_timestamp);

        // Map coordinates to cell indices (assuming cell size is 20m)
        int cell_x = static_cast<int>(x / 20.0);
        int cell_y = static_cast<int>(y / 20.0);

        // Ensure indices are within bounds (0 to 299)
        if (cell_x >= 0 && cell_x < 300 && cell_y >= 0 && cell_y < 300) {
            std::pair<int, int> cell_coord = std::make_pair(cell_x, cell_y);
            //std::cout << "Cell with coordinates (" << cell_x << ", " << cell_y << ") visited at time " << format_time_point(timestamp) << std::endl;
            cell_visit_times[cell_coord].push_back(timestamp);
        } else {
            std::cerr << "Invalid cell coordinates: (" << cell_x << ", " << cell_y << ") for position (" << x << ", " << y << ")" << std::endl;
        }
    }

    // Sort visit times for each cell
    for (auto &cell_entry : cell_visit_times) {
        std::sort(cell_entry.second.begin(), cell_entry.second.end());
    }
}



// Function to analyze cell visits
void analyze_cell_visits(const std::map<std::pair<int, int>, std::vector<std::chrono::system_clock::time_point>> &cell_visit_times,
                         const std::chrono::system_clock::time_point &simulation_start_time,
                         const std::chrono::system_clock::time_point &simulation_end_time,
                         const std::chrono::minutes &max_interval) {
    bool all_cells_ok = true;

    // Iterate over all possible cells in the grid
    for (int x = 0; x < 40; ++x) {
        for (int y = 0; y < 40; ++y) {
            std::pair<int, int> cell_coord = std::make_pair(x, y);
            auto it = cell_visit_times.find(cell_coord);

            if (it == cell_visit_times.end()) {
                // Cell was never visited
                all_cells_ok = false;
                std::cout << "Cell (" << x << ", " << y << ") was never visited during the simulation." << std::endl;
            } else {
                const auto &visits = it->second;

                auto interval = std::chrono::duration_cast<std::chrono::minutes>(visits.front() - simulation_start_time);
                // Check intervals between visits
                for (size_t i = 1; i < visits.size(); ++i) {
                    interval = std::chrono::duration_cast<std::chrono::minutes>(visits[i] - visits[i - 1]);
                    if (interval >= max_interval) {
                        all_cells_ok = false;
                        std::cout << "Cell (" << x << ", " << y << ") was not visited for " << interval.count() << " minutes between " << format_time_point(visits[i - 1]) << " and " << format_time_point(visits[i]) << "." << std::endl;
                    }
                }

                // Check from last visit to simulation end
                interval = std::chrono::duration_cast<std::chrono::minutes>(simulation_end_time - visits.back());
                if (interval >= max_interval) {
                    all_cells_ok = false;
                    std::cout << "Cell (" << x << ", " << y << ") was not visited in the last " << max_interval.count() << " minutes before simulation end." << std::endl;
                    std::cout << "Last visit at: " << format_time_point(visits.back()) << std::endl;
                }
            }
        }
    }

    if (all_cells_ok) {
        std::cout << "All cells were visited within every " << max_interval.count() << "-minute interval." << std::endl;
    }
}


int main() {
    // Initialize Redis connection
    RedisCommunication redis_comm("127.0.0.1", 6379);
    auto redis = redis_comm.get_redis_instance();

    const std::chrono::minutes max_interval(5);

    // Data structures to hold visit times and simulation time frame
    std::vector<nlohmann::json> status_logs;
    std::map<std::pair<int, int>, std::vector<std::chrono::system_clock::time_point>> cell_visit_times;
    std::chrono::system_clock::time_point simulation_start_time;
    std::chrono::system_clock::time_point simulation_end_time;

    // Retrieve status logs from Redis
    retrieve_status_logs_from_redis(redis, status_logs, simulation_start_time, simulation_end_time);

    if (status_logs.empty()) {
        std::cerr << "No status logs found in Redis." << std::endl;
        return 1;
    }

    // Parse the status logs
    parse_status_logs(status_logs, cell_visit_times);

    if (simulation_start_time == std::chrono::system_clock::time_point::min() ||
        simulation_end_time == std::chrono::system_clock::time_point::min()) {
        std::cerr << "No valid timestamps found in the status logs." << std::endl;
        return 1;
    }

    // Analyze the cell visits
    analyze_cell_visits(cell_visit_times, simulation_start_time, simulation_end_time, max_interval);

    // After analysis
    redis->del("status_logs");


    return 0;
}
