// WatchZone.cpp
#include "WatchZone.h"

std::vector<std::shared_ptr<Sector>> WatchZone::createSectors() {
    float cellSize = 20; // Assuming 20m x 20m cells
    size_t cellsPerSector = 10; // 10x10 cells per sector
    auto sectorSize = static_cast<size_t>(cellsPerSector * cellSize);

    auto numRows = static_cast<std::size_t>(std::ceil(this->height / cellSize));
    auto numCols = static_cast<std::size_t>(std::ceil(this->width / cellSize));

    std::vector allCells(numRows, std::vector<std::shared_ptr<Cell>>(numCols)); // Vector not needed use std::array instead

    for (int i = 0; i < height / cellSize; i++) {
        for (int j = 0; j < width / cellSize; j++) {
            allCells[i][j] = std::make_shared<Cell>(j * cellSize, (j + 1) * cellSize, i * cellSize, (i + 1) * cellSize);
        }
    }

    std::vector<std::shared_ptr<Sector>> sectors;
    sectors.reserve(9000);  // Change this to be dynamic
    int sectorID = 0;
    for (size_t y = 0; y < height; y += sectorSize) {
        for (size_t x = 0; x < width; x += cellsPerSector * cellSize) {
            int startX = static_cast<int>(x / cellSize);
            int startY = static_cast<int>(y / cellSize);
            sectors.emplace_back(std::make_shared<Sector>(sectorID++, startX, startY, allCells));
        }
    }

    return sectors;
}

sf::Vector2f scalePosition(double x, double y) {
    // Assuming your coordinate system ranges from (0,0) to (6000,6000)
    // Adjust the scaling to fit the window size

    const double grid_width = 6000.0;
    const double grid_height = 6000.0;
    const int window_width = 800;
    const int window_height = 800;

    double scale_x = window_width / grid_width;
    double scale_y = window_height / grid_height;

    float screen_x = static_cast<float>(x * scale_x);
    float screen_y = static_cast<float>(y * scale_y);

    // SFML's y-axis increases downward, which matches your coordinate system

    return sf::Vector2f(screen_x, screen_y);
}

void WatchZone::drawGrid(sf::RenderWindow& window) {
    sf::Color gridColor = sf::Color(200, 200, 200); // Light gray color for grid lines

    // Create vertical lines
    for (int i = 0; i <= gridCols; ++i) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(i * cellWidth, 0), gridColor),
            sf::Vertex(sf::Vector2f(i * cellWidth, windowHeight), gridColor)
        };
        window.draw(line, 2, sf::Lines);
    }

    // Create horizontal lines
    for (int i = 0; i <= gridRows; ++i) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(0, i * cellHeight), gridColor),
            sf::Vertex(sf::Vector2f(windowWidth, i * cellHeight), gridColor)
        };
        window.draw(line, 2, sf::Lines);
    }
}

void WatchZone::visualizationThread(TowerClient& tower_client) {
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Drone Monitoring");

    // Load font for text (make sure the font file is available)
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error loading font." << std::endl;
        // Handle error appropriately
    }

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Clear the window
        window.clear(sf::Color::White);

        // Draw the grid
        drawGrid(window);

        // Optional: Highlight sectors
        //drawSectorHighlights(window, sectors);

        // Optional: Draw sector labels
        //drawSectorLabels(window, font);

        // Get the latest drone statuses
        auto drone_statuses = tower_client.get_drone_statuses();

        // Process and draw drone positions
        for (const auto& [drone_id, status] : drone_statuses) {
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
            if (battery < 20.0) {
                drone_shape.setFillColor(sf::Color::Red);
            }

            // Draw the drone
            window.draw(drone_shape);

            // Optionally, display the drone's ID
            // sf::Text drone_text;
            // drone_text.setFont(font);
            // drone_text.setString(std::to_string(drone_id));
            // drone_text.setCharacterSize(12);
            // drone_text.setFillColor(sf::Color::Black);
            // drone_text.setPosition(position.x + 5, position.y + 5);

            // window.draw(drone_text);
        }

        // Display the window
        window.display();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

WatchZone::WatchZone()
        : sectors(createSectors()), // Initialize sectors using the new method
          cerebrum(sectors), // Initialize cerebrum with the newly created sectors
          redisCommunication("127.0.0.1", 6379),
          client(redisCommunication.get_redis_instance(), sectors)
{
    // Listen for drone connections
    client.start_listening_for_drones();
    std::cout << "Listening for drone connections..." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    // Look for disconnected drones
    client.start_monitoring_drones();
    std::cout << "Monitoring drones..." << std::endl;

    visualizationThread(std::ref(client));
}



// wtf?? need to implement redis communication for this
Position WatchZone::getDronePosition(int droneID) {
    // query redis to get the position

    return drones[droneID].getPosition();
}

// This function calculates the area size of the region that a single drone can cover in 5 minutes
[[maybe_unused]] double getRegionArea() {
    // Calculate the distance that a drone can cover in 5 minutes
    // double distance = Drone::getSpeed() * 5 / 60; // Speed * Time = Distance (in km) --- Not needed in this case

    // TSP algorithm takes an exponential amount of time to compute, so we either keep the number of cells small or pre-compute the paths
    // For simplicity, we will keep the number of cells small for each region (e.g., 10x10 grid)
    // Moving from the center of a cell to the center of another cell is equivalent to moving from the top-left corner of a cell to the top-left corner of another cell
    double droneCoverageArea = Drone::getVisibilityRange() * 2;
    droneCoverageArea *= droneCoverageArea; // Area = Side * Side
    return 10 * 10 * droneCoverageArea; // 10x10 grid of cells
}