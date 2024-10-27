#include "Cerebrum.h"

using namespace operations_research;


Cerebrum::Cerebrum(const std::vector<std::shared_ptr<Sector>> &s) : sectors(s) {
    const Sector *sector = sectors[464].get();
    // Solve TSP for the first sector
    solveTSP(sector->getWaypoints(), sector->getStartingIndex());
    for (const auto &sector : sectors) {

    }
}

void visualizeTour(const std::vector<RoutingNodeIndex>& tour, const std::array<Position, 100>& cell_positions) {
    // Set up the window
    const int window_width = 800;
    const int window_height = 800;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "TSP Path Visualization");

    // Determine the min and max coordinates
    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::lowest();

    for (const auto& pos : cell_positions) {
        if (pos.x < min_x) min_x = pos.x;
        if (pos.x > max_x) max_x = pos.x;
        if (pos.y < min_y) min_y = pos.y;
        if (pos.y > max_y) max_y = pos.y;
    }

    // Add some padding
    double padding = 10.0;
    min_x -= padding;
    max_x += padding;
    min_y -= padding;
    max_y += padding;

    // Compute the scaling factors
    double scale_x = window_width / (max_x - min_x);
    double scale_y = window_height / (max_y - min_y);

    // Function to scale positions to window coordinates
    auto scalePosition = [&](const Position& pos) -> sf::Vector2f {
        return sf::Vector2f(
                static_cast<float>((pos.x - min_x) * scale_x),
                static_cast<float>((pos.y - min_y) * scale_y)
        );
    };

    // Create shapes for the cells
    std::vector<sf::CircleShape> cell_shapes;
    float cell_radius = 3.0f;

    for (const auto& pos : cell_positions) {
        sf::CircleShape shape(cell_radius);
        sf::Vector2f scaled_pos = scalePosition(pos);
        //  scaled_pos.y = window_height - scaled_pos.y; // Invert y-axis if necessary
        shape.setPosition(scaled_pos - sf::Vector2f(cell_radius, cell_radius));
        shape.setFillColor(sf::Color::Blue);
        cell_shapes.push_back(shape);
    }

    // Extract the positions in the order of the tour
    std::vector<Position> tour_positions;
    for (RoutingNodeIndex node_index : tour) {
        tour_positions.push_back(cell_positions[node_index.value()]);
    }

    // Create vertices for the path
    std::vector<sf::Vertex> path_vertices;
    for (size_t i = 0; i < tour_positions.size(); ++i) {
        sf::Vector2f scaled_pos = scalePosition(tour_positions[i]);
        scaled_pos.y = window_height - scaled_pos.y; // Invert y-axis if necessary
        path_vertices.push_back(sf::Vertex(scaled_pos, sf::Color::Red));
    }

    // Close the loop
    path_vertices.push_back(path_vertices.front());

    // Highlight the starting point
    RoutingNodeIndex start_node_index = tour.front();
    cell_shapes[start_node_index.value()].setFillColor(sf::Color::Green);
    cell_shapes[start_node_index.value()].setRadius(cell_radius * 1.5f);

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);

        // Draw the path
        if (!path_vertices.empty()) {
            window.draw(&path_vertices[0], path_vertices.size(), sf::LineStrip);
        }

        // Draw the cells
        for (const auto& shape : cell_shapes) {
            window.draw(shape);
        }

        window.display();
    }
}


std::array<std::array<int, 100>, 100> Cerebrum::ComputeDistanceMatrix(const std::array<Position, 100> &positions) {
    const int size = positions.size();
    std::array<std::array<int, 100>, 100> distance_matrix{};
    for (int from = 0; from < size; ++from) {
        for (int to = 0; to < size; ++to) {
            if (from == to) continue;
            double dx = positions[from].x - positions[to].x;
            double dy = positions[from].y - positions[to].y;
            double distance = std::sqrt(dx * dx + dy * dy);
            distance_matrix[from][to] = static_cast<int>(distance * 1000); // Scale if needed.
        }
    }
    return distance_matrix;
}

void Cerebrum::solveTSP(const std::array<Position, 100> &positions, int starting_index) {
    RoutingNodeIndex start_index(starting_index);

    // Print the positions
    for (size_t i = 0; i < positions.size(); ++i) {
        const auto& [x, y] = positions[i];
        std::cout << "Cell " << i << ": (" << x << ", " << y << ")\n";
    }

    const auto distance_matrix = ComputeDistanceMatrix(positions);
    RoutingIndexManager manager(static_cast<int>(positions.size()), 1, start_index);
    RoutingModel routingModel(manager);

    const int transit_callback_index = routingModel.RegisterTransitCallback(
            [&distance_matrix, &manager](int64_t from_index, int64_t to_index) -> int64_t {
                RoutingNodeIndex from_node = manager.IndexToNode(from_index);
                RoutingNodeIndex to_node = manager.IndexToNode(to_index);
                return distance_matrix[from_node.value()][to_node.value()];
            }
    );
    routingModel.SetArcCostEvaluatorOfAllVehicles(transit_callback_index);

    RoutingSearchParameters search_parameters = DefaultRoutingSearchParameters();
    search_parameters.set_first_solution_strategy(FirstSolutionStrategy::PATH_CHEAPEST_ARC);
    search_parameters.set_local_search_metaheuristic(LocalSearchMetaheuristic::GUIDED_LOCAL_SEARCH);
    search_parameters.mutable_time_limit()->set_seconds(3);

    const Assignment* solution = routingModel.SolveWithParameters(search_parameters);

    if (solution != nullptr) {
        std::vector<RoutingNodeIndex> tour;
        int64_t index = routingModel.Start(0);
        while (!routingModel.IsEnd(index)) {
            RoutingNodeIndex node_index = manager.IndexToNode(index);
            tour.push_back(node_index);
            index = solution->Value(routingModel.NextVar(index));
        }
        tour.push_back(start_index);

        visualizeTour(tour, positions);
        // Output the tour
        for (RoutingNodeIndex node : tour) {
            Position pos = positions[node.value()];
            std::cout << "Visit cell " << node << " at position ("
                      << pos.x << ", " << pos.y << ")\n";
        }

        // Apply transformations for other sectors
        // For example, mirror over vertical axis and translate
        // std::array<Position, 100> transformed_positions{};
        // for (RoutingNodeIndex node : tour) {
        //     Position pos = positions[node.value()];
        //     pos = MirrorOverVerticalAxis(pos, /* axis_x */);
        //     pos = TranslatePosition(pos, /* dx */, /* dy */);
        //     transformed_positions.push_back(pos);
        // }

        // Output the transformed tour
        // std::cout << "Transformed tour:\n";
        // for (const auto& pos : transformed_positions) {
        //     std::cout << "Visit position (" << pos.x << ", " << pos.y << ")\n";
        // }

    } else {
        std::cout << "No solution found.\n";
    }

}
