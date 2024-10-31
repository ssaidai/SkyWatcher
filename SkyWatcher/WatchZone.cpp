// WatchZone.cpp
#include "WatchZone.h"

std::vector<std::shared_ptr<Sector>> WatchZone::createSectors()
{
    logInfo("Tower", "Creating sectors...");
    constexpr float cellSize = 20; // Assuming 20m x 20m cells
    constexpr size_t cellsPerSector = 10; // 10x10 cells per sector
    constexpr auto sectorSize = static_cast<size_t>(cellsPerSector * cellSize);

    this->numRows = static_cast<std::size_t>(std::ceil(this->height / cellSize));
    this->numCols = static_cast<std::size_t>(std::ceil(this->width / cellSize));

    std::vector allCells(numRows, std::vector<std::shared_ptr<Cell>>(numCols));
    // Vector not needed use std::array instead

    for (int i = 0; i < height / cellSize; i++)
    {
        for (int j = 0; j < width / cellSize; j++)
        {
            allCells[i][j] = std::make_shared<Cell>(j * cellSize, (j + 1) * cellSize, i * cellSize, (i + 1) * cellSize);
        }
    }

    this->numCols/=10;
    this->numRows/=10;

    std::vector<std::shared_ptr<Sector>> sectors;
    sectors.reserve(numCols * numRows); // Change this to be dynamic
    int sectorID = 0;
    for (size_t y = 0; y < height; y += sectorSize)
    {
        for (size_t x = 0; x < width; x += cellsPerSector * cellSize)
        {
            int startX = static_cast<int>(x / cellSize);
            int startY = static_cast<int>(y / cellSize);
            sectors.emplace_back(std::make_shared<Sector>(sectorID++, startX, startY, allCells, this->height));
        }
    }
    logInfo("Tower", "Sectors created");
    return sectors;
}

sf::Vector2f WatchZone::scalePosition(const double x, const double y) const
{
    // Assuming your coordinate system ranges from (0,0) to (6000,6000)
    // Adjust the scaling to fit the window size

    const double grid_width = this->width;
    const double grid_height = this->height;

    const double scale_x = windowWidth / grid_width;
    const double scale_y = windowHeight / grid_height;

    auto screen_x = static_cast<float>(x * scale_x);
    auto screen_y = static_cast<float>(y * scale_y);

    // SFML's y-axis increases downward, which matches your coordinate system

    return {screen_x, screen_y};
}

void WatchZone::drawGrid(sf::RenderWindow& window) const
{
    // Draw cells' line
    const auto cellColor = sf::Color(200, 200,200); // Light gray color for grid lines
    // Create vertical lines
    for (int i = 0; i <= numCols*10; ++i)
    {
        const sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(i * cellWidth/10, 0), cellColor),
            sf::Vertex(sf::Vector2f(i * cellWidth/10, windowHeight), cellColor)
        };
        window.draw(line, 2, sf::Lines);
    }

    // Create horizontal lines
    for (int i = 0; i <= numRows*10; ++i)
    {
        const sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(0, i * cellHeight/10), cellColor),
            sf::Vertex(sf::Vector2f(windowWidth, i * cellHeight/10), cellColor)
        };
        window.draw(line, 2, sf::Lines);
    }

    // Draw sectors' line
    const auto sectorColor = sf::Color(0, 0, 0); // Light gray color for grid lines
    // Create vertical lines
    for (int i = 0; i <= numCols; ++i)
    {
        const sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(i * cellWidth, 0), sectorColor),
            sf::Vertex(sf::Vector2f(i * cellWidth, windowHeight), sectorColor)
        };
        window.draw(line, 2, sf::Lines);
    }

    // Create horizontal lines
    for (int i = 0; i <= numRows; ++i)
    {
        const sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(0, i * cellHeight), sectorColor),
            sf::Vertex(sf::Vector2f(windowWidth, i * cellHeight), sectorColor)
        };
        window.draw(line, 2, sf::Lines);
    }
}

void WatchZone::drawPoints(sf::RenderWindow& window, const std::vector<std::shared_ptr<Sector>>& sectors) const {
    // Draw tower point
    sf::Color towerPointColor = sf::Color::Black; // Color for starting points
    Position towerPoint {(this->width/2.0), this->height/2.0};
    sf::Vector2f position = scalePosition(towerPoint.x, towerPoint.y);

    // Create a square shape for the starting point
    float size = 8.5f;
    sf::RectangleShape tower_point_shape(sf::Vector2f(size, size));
    tower_point_shape.setOrigin(size / 2.0f, size / 2.0f); // Center the shape
    tower_point_shape.setPosition(position);
    tower_point_shape.setFillColor(towerPointColor);

    window.draw(tower_point_shape);


    // Draw starting points
    sf::Color startingPointColor = sf::Color::Green; // Color for starting points
    for (const auto& sector : sectors) {
        auto [x, y] = sector->getStartingPoint();
        sf::Vector2f position = scalePosition(x, y);

        // Create a square shape for the starting point
        float size = 10.0f;
        sf::RectangleShape starting_point_shape(sf::Vector2f(size, size));
        starting_point_shape.setOrigin(size / 2.0f, size / 2.0f); // Center the shape
        starting_point_shape.setPosition(position);
        starting_point_shape.setFillColor(startingPointColor);

        window.draw(starting_point_shape);
    }
}


void WatchZone::visualizationThread(TowerClient& tower_client, const std::vector<std::shared_ptr<Sector>>& sectors) const
{
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Drone Monitoring");

    while (window.isOpen())
    {
        sf::Event event{};
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Clear the window
        window.clear(sf::Color::White);

        // Draw the grid
        drawGrid(window);


        drawPoints(window, sectors);
        // Optional: Highlight sectors
        //drawSectorHighlights(window, sectors);

        // Optional: Draw sector labels
        //drawSectorLabels(window, font);

        // Get the latest drone statuses
        auto drone_statuses = tower_client.get_drone_statuses();

        // Process and draw drone positions
        for (const auto& [drone_id, status] : drone_statuses)
        {
            double x = status["position"]["x"];
            double y = status["position"]["y"];
            double battery = status["battery_level"];

            // Scale positions to window size
            sf::Vector2f position = scalePosition(x, y);

            // Create a circle to represent the drone
            sf::CircleShape drone_shape(5.0f);
            drone_shape.setOrigin(5.0f, 5.0f); // Center the shape
            drone_shape.setPosition(position);
            drone_shape.setFillColor(sf::Color::Blue);

            // Optionally change color based on battery level
            if (battery == 0.0)
            {
                drone_shape.setFillColor(sf::Color::Red);
            }

            // Draw the drone
            window.draw(drone_shape);
        }

        // Display the window
        window.display();

        std::this_thread::sleep_for(std::chrono::duration<float>(0.1 / timeScale));
    }
}

WatchZone::WatchZone(const int areaSize, const int timeScale = 1)
    : width(areaSize), height(areaSize), timeScale(timeScale),
      sectors(createSectors()), // Initialize sectors using the new method
      cerebrum(sectors), // Initialize cerebrum with the newly created sectors
      redisCommunication("127.0.0.1", 6379),
      client(redisCommunication.get_redis_instance(), sectors, timeScale, center)
{
    // Listen for drone connections
    client.start_listening_for_drones();
    std::cout << "Listening for drone connections..." << std::endl;
    logInfo("Tower", "Start listening for drone connection...");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    // Look for disconnected drones
    client.start_monitoring_drones();
    std::cout << "Monitoring drones..." << std::endl;
    logInfo("Tower", "Start monitoring drones...");

    client.start_substitution_listener();
    std::cout << "Listening for substitution messages..." << std::endl;
    logInfo("Tower", "Start listening for drone substitution requests...");

    visualizationThread(std::ref(client), sectors);
}
