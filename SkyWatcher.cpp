#include <SFML/Graphics.hpp>
#include "Cerebrum.h"
#include "Drone.h"
#include "WatchZone.h"

// Function prototypes
void updateDronePositions(std::vector<sf::VertexArray>& dronePaths, WatchZone& watchZone);

int main()
{
    WatchZone watchZone;

    int NUM_DRONES = watchZone.droneCount;
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(600, 600), "Drone Surveillance System");

    sf::View view(sf::FloatRect(0, 0, 600, 600));
    view.setCenter(6000, 6000);  // Center on the initial position of the drones
    view.zoom(0.1f);  // Scale the view to fit the 6000x6000 area into the 800x600 window
    window.setView(view);

    // Create a vector to store drone positions
    std::vector<sf::VertexArray> dronePaths;
    for (int i = 0; i < NUM_DRONES; ++i)
    {
        dronePaths.push_back(sf::VertexArray(sf::LinesStrip));
    }

    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Update drone positions from your system
        updateDronePositions(dronePaths, watchZone);

        // Clear screen
        window.clear();

        // Draw drone paths
        for (const auto& path : dronePaths)
        {
            window.draw(path);
        }

        // Update the window
        window.display();
    }

    return 0;
}

void updateDronePositions(std::vector<sf::VertexArray>& dronePaths, WatchZone& watchZone)
{
    for (size_t i = 0; i < dronePaths.size(); ++i)
    {
        // Get current drone position from your system
        Position currentPos = watchZone.getDronePosition(i);

        // Convert to SFML coordinates and add to path
        sf::Vertex point(sf::Vector2f(currentPos.x, currentPos.y));
        dronePaths[i].append(point);
    }
}
