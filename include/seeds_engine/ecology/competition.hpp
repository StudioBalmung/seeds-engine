#pragma once
#include <algorithm>

namespace seeds::ecology {

class CompetitionEngine {
public:
    template <typename Tile>
    [[nodiscard]] double population_pressure(const Tile& tile) const {
        return static_cast<double>(tile.occupants.size()) / std::max(1, tile.carrying_capacity());
    }
};

} // namespace seeds::ecology
