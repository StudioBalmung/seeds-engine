#pragma once
#include <algorithm>
#include <random>
#include <string>

#include "../behavior/perception.hpp"
#include "../life/organism.hpp"
#include "../life/species.hpp"

namespace seeds::ecology {

class PredationEngine {
public:
    template <typename Engine>
    bool resolve(Engine& engine, life::Organism& predator,
                 const life::SpeciesProfile& predator_species,
                 const behavior::PerceptionSnapshot& perception) {
        std::vector<std::string> valid_prey;
        for (const auto& prey_id : perception.nearby_prey_ids) {
            if (engine.organisms.count(prey_id)) valid_prey.push_back(prey_id);
        }
        if (valid_prey.empty()) {
            predator.energy = std::max(0.0, predator.energy - 0.05);
            return false;
        }

        // Pick weakest prey
        std::string target_id = valid_prey[0];
        double min_health = engine.organisms[valid_prey[0]].health;
        for (const auto& pid : valid_prey) {
            if (engine.organisms[pid].health < min_health) {
                min_health = engine.organisms[pid].health;
                target_id = pid;
            }
        }

        auto& prey = engine.organisms[target_id];
        double success_chance = std::min(0.95,
            0.45 + predator_species.predation_power * 0.35 + predator.phenotype.adaptation_score * 0.15);

        std::uniform_real_distribution<double> dist(0.0, 1.0);
        if (dist(engine.rng) <= success_chance) {
            prey.health -= 0.65 + predator_species.predation_power * 0.2;
            predator.hunger = std::max(0.0, predator.hunger - 0.8);
            predator.energy = std::min(1.0, predator.energy + 0.3);
            if (prey.health <= 0.0) {
                engine.mark_death(prey.organism_id, "predation:" + predator.organism_id);
                predator.energy = std::min(1.0, predator.energy + 0.1);
            }
            return true;
        }

        predator.energy = std::max(0.0, predator.energy - 0.1);
        return false;
    }
};

} // namespace seeds::ecology
