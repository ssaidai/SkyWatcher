

#include <iostream>
#include <vector>
#include "ortools/constraint_solver/routing.h"
#include "ortools/constraint_solver/routing_parameters.h"
#include "ortools/constraint_solver/constraint_solver.h"

using namespace operations_research;

// Define the grid size
const int GRID_SIZE = 10;

// Function to compute Manhattan distance between two nodes in the grid
int ManhattanDistance(int from_node, int to_node) {
    int x1 = from_node / GRID_SIZE;
    int y1 = from_node % GRID_SIZE;
    int x2 = to_node / GRID_SIZE;
    int y2 = to_node % GRID_SIZE;
    return abs(x1 - x2) + abs(y1 - y2);
}

// Create the distance matrix for the grid
std::vector<std::vector<int>> CreateDistanceMatrix(int grid_size) {
    std::vector<std::vector<int>> distance_matrix(grid_size * grid_size, std::vector<int>(grid_size * grid_size));
    for (int from_node = 0; from_node < grid_size * grid_size; ++from_node) {
        for (int to_node = 0; to_node < grid_size * grid_size; ++to_node) {
            distance_matrix[from_node][to_node] = ManhattanDistance(from_node, to_node);
        }
    }
    return distance_matrix;
}

// Callback to return distance between two nodes
class DistanceMatrixCallback {
public:
    DistanceMatrixCallback(const std::vector<std::vector<int>>& distance_matrix)
            : distance_matrix_(distance_matrix) {}

    int operator()(int from_index, int to_index) const {
        return distance_matrix_[from_index][to_index];
    }

private:
    const std::vector<std::vector<int>>& distance_matrix_;
};

// Print the solution
void PrintSolution(const RoutingModel& routing, const RoutingIndexManager& manager, const Assignment& solution) {
    int index = routing.Start(0);
    int route_distance = 0;
    std::cout << "Route:\n";
    while (!routing.IsEnd(index)) {
        std::cout << manager.IndexToNode(index) << " -> ";
        int previous_index = index;
        index = solution.Value(routing.NextVar(index));
        route_distance += routing.GetArcCostForVehicle(previous_index, index, 0);
    }
    std::cout << manager.IndexToNode(index) << std::endl;
    std::cout << "Distance of the route: " << route_distance << " units" << std::endl;
}

int main() {
    // Instantiate the data problem
    std::vector<std::vector<int>> distance_matrix = CreateDistanceMatrix(GRID_SIZE);

    // Create Routing Index Manager
    RoutingIndexManager manager(GRID_SIZE * GRID_SIZE, 1, RoutingNodeIndex(0));  // Single vehicle, starting from node 0

    // Create Routing Model
    RoutingModel routing(manager);

    // Create and register a transit callback
    DistanceMatrixCallback distance_callback(distance_matrix);
    int transit_callback_index = routing.RegisterTransitCallback([&distance_callback](int64_t from_index, int64_t to_index) -> int64_t {
        return distance_callback(from_index, to_index);
    });

    // Set the cost of travel (distance)
    routing.SetArcCostEvaluatorOfAllVehicles(transit_callback_index);

    // Define the search parameters
    RoutingSearchParameters search_parameters = DefaultRoutingSearchParameters();
    search_parameters.set_first_solution_strategy(FirstSolutionStrategy::PATH_CHEAPEST_ARC);

    // Solve the problem
    const Assignment* solution = routing.SolveWithParameters(search_parameters);

    // Print solution if found
    if (solution != nullptr) {
        PrintSolution(routing, manager, *solution);
    } else {
        std::cout << "No solution found!" << std::endl;
    }

    return 0;
}