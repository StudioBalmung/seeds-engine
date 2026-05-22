#pragma once
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "disease.hpp"
#include "faith.hpp"
#include "genetics.hpp"
#include "species.hpp"

namespace seeds::life {

struct Organism {
    std::string organism_id;
    std::string species_id;
    std::string sex;
    std::pair<int, int> position = {0, 0};
    Genome genome;
    Phenotype phenotype;
    int age_ticks = 0;
    double fertility = 0.0;
    double body_mass = 1.0;
    double hunger = 0.0;
    double health = 1.0;
    double energy = 1.0;
    double thirst = 0.0;
    int lifespan_ticks = 0;
    DiseaseState disease;
    FaithState faith;
    double sensory_capability = 1.0;
    int movement_capability = 1;
    bool alive = true;
    int generation = 0;
    std::vector<std::string> parents;
    int offspring_count = 0;
    std::string behavior_state = "idle";
    std::string last_action = "spawned";
    int reproduction_cooldown = 0;

    [[nodiscard]] bool is_adult(const SpeciesProfile& species) const noexcept {
        return age_ticks >= species.mature_age_ticks;
    }

    [[nodiscard]] bool ready_to_reproduce(const SpeciesProfile& species) const noexcept {
        return alive
            && is_adult(species)
            && reproduction_cooldown == 0
            && energy >= species.reproduction_energy_threshold
            && health >= species.reproduction_health_threshold
            && hunger <= species.reproduction_hunger_limit
            && thirst <= species.reproduction_thirst_limit
            && fertility >= 0.65;
    }
};

} // namespace seeds::life
