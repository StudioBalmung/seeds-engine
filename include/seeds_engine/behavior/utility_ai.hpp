#pragma once
#include "actions.hpp"
#include "perception.hpp"
#include "../life/organism.hpp"
#include "../life/species.hpp"

namespace seeds::behavior {

class UtilityBehavior {
public:
    [[nodiscard]] ActionDecision select_action(const life::Organism& organism,
                                               const life::SpeciesProfile& species,
                                               const PerceptionSnapshot& perception) const {
        if (!perception.nearby_predator_ids.empty() && species.diet == life::DietType::Herbivore) {
            return {ActionType::Flee, "predator detected"};
        }
        if (organism.thirst >= 0.55 && perception.tile_water > 1.5) {
            return {ActionType::Drink, "water pressure"};
        }
        if (species.diet == life::DietType::Herbivore && organism.hunger >= 0.45 && perception.tile_plants > 2.0) {
            return {ActionType::Graze, "plant forage available"};
        }
        if (species.can_predate() && organism.hunger >= 0.45 && !perception.nearby_prey_ids.empty()) {
            return {ActionType::Hunt, "prey nearby"};
        }
        if (organism.ready_to_reproduce(species) && !perception.nearby_mate_ids.empty()) {
            return {ActionType::SeekMate, "mate ready"};
        }
        if (organism.energy <= 0.35 || (!perception.is_daylight && organism.hunger < 0.45)) {
            return {ActionType::Rest, "recovery"};
        }
        return {ActionType::Wander, "exploration"};
    }
};

} // namespace seeds::behavior
