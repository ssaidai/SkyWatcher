#include "Drone/Drone.h"


int main() {
    // Initialize a drone
    std::vector<std::thread> threads;
    for(int i = 0; i < 16; i++) {
        threads.emplace_back([]() {
            Drone drone(60);
        });
    }
    for(auto& thread : threads) {
        thread.join();
    }
    return 0;
}