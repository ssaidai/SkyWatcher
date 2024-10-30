#ifndef SKYWATCHER_WATCHZONE_H
#define SKYWATCHER_WATCHZONE_H

#include <vector>
#include "Drone/Drone.h"
#include "Cerebrum.h"
#include "Redis/Redis.h"
#include "../Utils/Logger.h"

// SkyWatcher class using grid implementation and TSP algorithm
class WatchZone {
private:
    int width, height, timeScale;
    Position center = {static_cast<double>(width/2), static_cast<double>(height/2)};
    std::vector<std::shared_ptr<Sector>> sectors;
    std::vector<Drone> drones;
    Cerebrum cerebrum;
    RedisCommunication redisCommunication;
    TowerClient client;
    std::vector<std::shared_ptr<Sector>> createSectors();
    sf::Vector2f scalePosition(double x, double y) const;
    void drawGrid(sf::RenderWindow& window) const;
    void drawPoints(sf::RenderWindow& window, const std::vector<std::shared_ptr<Sector>>& sectors) const;
    void visualizationThread(TowerClient &client, const std::vector<std::shared_ptr<Sector>>& sectors) const;

    int numRows, numCols;

    // Window dimensions (should match the window you create)
    constexpr static int windowWidth = 800;
    constexpr static int windowHeight = 800;

    // Calculate cell size
    float cellWidth = static_cast<float>(windowWidth) / numCols;
    float cellHeight = static_cast<float>(windowHeight) / numRows;
public:
    WatchZone(int areaSize, int timeScale);
};


#endif //SKYWATCHER_WATCHZONE_H
