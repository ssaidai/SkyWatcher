#include "Drone/Drone.h"


int main(const int argc, char* argv[]) {
    // Get command line arguments
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [timeScale]" << std::endl;
        return 1;
    }
    const int timeScale = std::stoi(argv[1]);
    if (timeScale <= 0) {
        std::cerr << "Invalid time scale. Please provide a positive integer." << std::endl;
        return 1;
    }
    // Initialize a drone
    std::vector<std::thread> threads;
    for(int i = 0; i < 36*8; i++) {
        threads.emplace_back([&timeScale]() {
            if(timeScale)
                Drone drone(timeScale);
            else
                Drone drone;
        });
    }
    for(auto& thread : threads) {
        thread.join();
    }
    return 0;
}