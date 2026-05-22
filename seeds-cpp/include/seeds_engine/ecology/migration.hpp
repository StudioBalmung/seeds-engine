#pragma once
#include <algorithm>
#include <cmath>
#include <random>
#include <utility>
#include <vector>

#include "../life/organism.hpp"
#include "../life/species.hpp"
#include "../world/map.hpp"

namespace seeds::ecology {

class MigrationPlanner {
public:
    explicit MigrationPlanner(std::mt19937& rng) : rng_(rng) {}

    std::pair<int, int> choose_destination(world::WorldMap& world,
                                           const life::Organism& organism,
                                           const life::SpeciesProfile& species) {
        auto& current_tile = world.get_tile(organism.position.first, organism.position.second);
        auto neighbor_ptrs = world.neighbors(organism.position.first, organism.position.second);

        std::vector<world::WorldTile*> candidates;
        candidates.push_back(&current_tile);
        for (auto* n : neighbor_ptrs) candidates.push_back(n);

        auto score = [&](const world::WorldTile* tile) -> double {
            double comfort = temperature_comfort(tile->climate.temperature, species.ideal_temperature_range);
            double plant_score = (species.diet == life::DietType::Herbivore)
                                     ? tile->resources.plants
                                     : tile->resources.water * 0.1;
            double water_score = tile->resources.water * 0.15;
            double crowding_penalty = static_cast<double>(tile->occupants.size()) /
                                      std::max(1, tile->carrying_capacity());
            return plant_score * 0.05 + water_score + comfort * 4.0 - crowding_penalty * 2.0;
        };

        double best_score = score(candidates[0]);
        for (auto* c : candidates) {
            best_score = std::max(best_score, score(c));
        }

        std::vector<world::WorldTile*> best_tiles;
        for (auto* c : candidates) {
            if (std::abs(score(c) - best_score) < 1e-9) {
                best_tiles.push_back(c);
            }
        }

        std::uniform_int_distribution<int> dist(0, static_cast<int>(best_tiles.size()) - 1);
        auto* chosen = best_tiles[dist(rng_)];
        return {chosen->x, chosen->y};
    }

private:
    static double temperature_comfort(double temperature, std::pair<double, double> ideal_range) {
        auto [low, high] = ideal_range;
        if (low <= temperature && temperature <= high) return 1.0;
        if (temperature < low) return std::max(0.0, 1.0 - (low - temperature) / 20.0);
        return std::max(0.0, 1.0 - (temperature - high) / 20.0);
    }

    std::mt19937& rng_;
};

} // namespace seeds::ecology
