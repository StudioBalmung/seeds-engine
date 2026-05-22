#pragma once
#include <algorithm>
#include <cmath>
#include <map>
#include <string>

namespace seeds::ai {

class CarryingCapacityPredictor {
public:
    template <typename Engine>
    std::map<std::string, double> predict(Engine& engine) const {
        int total_capacity = 0;
        engine.world.for_each_tile([&](const auto& tile) {
            total_capacity += tile.carrying_capacity();
        });
        int total_alive = 0;
        for (const auto& [id, organism] : engine.organisms) {
            if (organism.alive) total_alive++;
        }
        double pressure = static_cast<double>(total_alive) / std::max(1, total_capacity);
        return {
            {"estimated_capacity", static_cast<double>(total_capacity)},
            {"current_pressure", std::round(pressure * 10000.0) / 10000.0},
        };
    }
};

} // namespace seeds::ai
