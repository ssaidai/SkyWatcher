#ifndef SKYWATCHER_CEREBRUM_H
#define SKYWATCHER_CEREBRUM_H

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "Structs.h"
#include "GridDefinitions.h"
#include <SFML/Graphics.hpp>
#include <ortools/constraint_solver/routing.h>
#include <ortools/constraint_solver/routing_enums.pb.h>
#include <ortools/constraint_solver/routing_index_manager.h>
#include <ortools/constraint_solver/routing_parameters.h>
#include <ortools/constraint_solver/constraint_solver.h>


class Cerebrum {
private:
    std::vector<std::shared_ptr<Sector>> sectors;
    std::array<std::array<Position, 100>, 4> relativeTSPPaths{};

    std::array<std::array<int, 100>, 100> ComputeDistanceMatrix(const std::array<Position, 100> &positions);
    void fillCheckPoints();
public:
    explicit Cerebrum(const std::vector<std::shared_ptr<Sector>> &sectors);
    // TSP solver implementation
    void solveTSP(const std::array<Position, 100> &positions, int starting_index);
};


#endif //SKYWATCHER_CEREBRUM_H
