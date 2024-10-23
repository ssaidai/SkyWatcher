#include "Cerebrum.h"

using namespace operations_research;


Cerebrum::Cerebrum(const std::vector<std::shared_ptr<Sector>> &sectors) {
    this->sectors = sectors;
    // Solve TSP for each sector
    for (const auto &sector : sectors) {

    }
}

void Cerebrum::fillCheckPoints() {
    // Fill the checkPoints array with the positions of all sectors
    for (size_t i = 0; i < 100; i++) {
        checkPoints[i] = sectors[i]->getCenter();
    }
}

void Cerebrum::solveTSP(const std::vector<std::unique_ptr<Sector>> &sectors) {

}
