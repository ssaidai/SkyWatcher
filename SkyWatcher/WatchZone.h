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
    void drawGrid(sf::RenderWindow& window);
    void visualizationThread(TowerClient &client);

    const int gridRows = 30;
    const int gridCols = 30;

    // Window dimensions (should match the window you create)
    const int windowWidth = 800;
    const int windowHeight = 800;

    // Calculate cell size
    float cellWidth = static_cast<float>(windowWidth) / gridCols;
    float cellHeight = static_cast<float>(windowHeight) / gridRows;
public:
    WatchZone();
    Position getDronePosition(int droneID);
    int droneCount = 9000;
};


#endif //SKYWATCHER_WATCHZONE_H
