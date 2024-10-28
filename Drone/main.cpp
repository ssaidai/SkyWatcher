#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>
#include "Drone/Drone.h"
#include "Redis/Redis.h"
#include "Utils/utils.h"

int main() {
    std::vector<std::thread> threads;

    for (int i = 0; i < 128; i++) {
        threads.emplace_back([]() {
            Drone drone;
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    return 0;
}