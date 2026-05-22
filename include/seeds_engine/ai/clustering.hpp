#pragma once
#include <map>
#include <string>

#include "../life/species.hpp"

namespace seeds::ai {

class SpeciesClusterer {
public:
    template <typename Engine>
    std::map<std::string, std::string> cluster(const Engine& engine) const {
        std::map<std::string, std::string> result;
        for (const auto& [species_id, species] : engine.species) {
            result[species_id] = life::diet_to_string(species.diet) + ":" +
                                 std::to_string(species.movement_range) + ":" +
                                 std::to_string(species.sensory_range);
        }
        return result;
    }
};

} // namespace seeds::ai
