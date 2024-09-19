#ifndef SKYWATCHER_CEREBRUM_H
#define SKYWATCHER_CEREBRUM_H

#include <unordered_map>
#include <vector>
#include <cstdint>
#include "Structs.h"
#include "GridDefinitions.h"
#include <ortools/constraint_solver/routing.h>
#include <ortools/constraint_solver/routing_enums.pb.h>
#include <ortools/constraint_solver/routing_index_manager.h>
#include <ortools/constraint_solver/routing_parameters.h>


class Cerebrum {
private:
    std::vector<std::vector<Position>> regionsTSPPath;
public:
    // TSP solver implementation
    void solveTSP(const std::vector<std::unique_ptr<Sector>> &sector);
};


#endif //SKYWATCHER_CEREBRUM_H
