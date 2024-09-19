#include "Cerebrum.h"

using namespace operations_research;

void Cerebrum::solveTSP(const std::vector<std::unique_ptr<Sector>> &sectors) {
    const int gridSize = 10;  // 10x10 grid
    const int numCells = gridSize * gridSize;  // Total number of cells

    // Create a RoutingIndexManager with numCells nodes, 1 vehicle, and depot at node 0 (top-left corner).
    RoutingIndexManager manager(numCells, 1, RoutingIndexManager::NodeIndex(0));

    // Create a RoutingModel using the manager.
    RoutingModel routingModel(manager);

    // Register a transit callback for step-by-step distance between cells.
    const int transitCallbackIndex = routingModel.RegisterTransitCallback(
            [&manager, gridSize](int64_t fromIndex, int64_t toIndex) -> int64_t {
                // Convert internal indices to grid positions.
                int fromNode = manager.IndexToNode(fromIndex).value();
                int toNode = manager.IndexToNode(toIndex).value();

                // Get (x, y) coordinates from node index.
                int fromRow = fromNode / gridSize;
                int fromCol = fromNode % gridSize;
                int toRow = toNode / gridSize;
                int toCol = toNode % gridSize;

                // Allow step-by-step traversal (adjacent cells only).
                // Distance is 1 for adjacent cells, large value otherwise to discourage jumps.
                if ((std::abs(fromRow - toRow) == 1 && fromCol == toCol) ||
                    (std::abs(fromCol - toCol) == 1 && fromRow == toRow)) {
                    return 1;  // Distance between adjacent cells.
                }
                return 1000;  // Penalize non-adjacent cells.
            }
    );

    // Set the arc cost evaluator to the step-by-step distance callback.
    routingModel.SetArcCostEvaluatorOfAllVehicles(transitCallbackIndex);

    // Define the search parameters.
    RoutingSearchParameters searchParameters = DefaultRoutingSearchParameters();
    searchParameters.set_first_solution_strategy(FirstSolutionStrategy::PATH_CHEAPEST_ARC);

    // Solve the problem.
    const Assignment *solution = routingModel.SolveWithParameters(searchParameters);

    // Output the solution, if found.
    if (solution != nullptr) {
        std::vector<Position> tspPath;
        int64_t index = routingModel.Start(0);

        while (!routingModel.IsEnd(index)) {
            // Convert internal index to node and compute its position in the grid.
            int nodeIndex = manager.IndexToNode(index).value();
            int row = nodeIndex / gridSize;
            int col = nodeIndex % gridSize;

            // Store the current cell's position.
            tspPath.push_back(Position(row, col));

            // Move to the next node.
            index = solution->Value(routingModel.NextVar(index));
        }
        // Store the path.
        this->tspPath = tspPath;
    }
}
