#ifndef SKYWATCHER_CEREBRUM_H
#define SKYWATCHER_CEREBRUM_H

#include <unordered_map>
#include <vector>
#include "Structs.h"
#include "GridDefinitions.h"

class Cerebrum {
private:
    std::unordered_map<int, Path> pathToSectors;
public:
    explicit Cerebrum(const std::vector<std::unique_ptr<Sector>> &sectors);

    Path getPathToSector(int sectorId);
};


#endif //SKYWATCHER_CEREBRUM_H
