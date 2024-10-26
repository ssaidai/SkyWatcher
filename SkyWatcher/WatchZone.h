#ifndef SKYWATCHER_WATCHZONE_H
#define SKYWATCHER_WATCHZONE_H

#include <vector>
#include "Drone/Drone.h"
#include "Cerebrum.h"
#include "Redis/Redis.h"

// SkyWatcher class using grid implementation and TSP algorithm
class WatchZone {
private:
    double width = 6000, height = 6000;
    Position center = {3000, 3000};
    std::vector<std::shared_ptr<Sector>> sectors;
    std::vector<Drone> drones;
    Cerebrum cerebrum;
    RedisCommunication redisCommunication;
    TowerClient client;
    std::vector<std::shared_ptr<Sector>> createSectors();
public:
    WatchZone();
    Position getDronePosition(int droneID);
    int droneCount = 9000;
};


#endif //SKYWATCHER_WATCHZONE_H
