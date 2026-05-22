#pragma once
#include <algorithm>
#include <cmath>

#include "aging.hpp"
#include "organism.hpp"
#include "species.hpp"

namespace seeds::life {

class PhysiologyEngine {
public:
    template <typename Tile>
    void apply_tick(Organism& organism, const SpeciesProfile& species, const Tile& tile) {
        organism.age_ticks += 1;
        AgeStage stage = aging_model_.resolve_stage(organism.age_ticks, species);
        double climate_stress = compute_climate_stress(tile.climate.temperature, species.ideal_temperature_range);

        organism.hunger += species.metabolism * (1.0 + climate_stress * 0.5);
        organism.thirst += species.water_need * (1.0 + climate_stress * 0.6);
        organism.energy -= species.metabolism * 0.2 + climate_stress * 0.08;

        if (stage == AgeStage::Infant || stage == AgeStage::Juvenile) {
            organism.body_mass += species.base_body_mass * 0.01;
        } else if (stage == AgeStage::Elder) {
            organism.energy -= 0.02;
        }

        if (organism.hunger > 1.0) {
            organism.health -= (organism.hunger - 1.0) * 0.05;
        }
        if (organism.thirst > 1.0) {
            organism.health -= (organism.thirst - 1.0) * 0.06;
        }
        if (organism.age_ticks > organism.lifespan_ticks) {
            organism.health -= 0.05;
        }
        int occupant_count = static_cast<int>(tile.occupants.size());
        if (occupant_count > tile.carrying_capacity()) {
            organism.health -= 0.015 * (occupant_count - tile.carrying_capacity());
        }

        if (organism.is_adult(species) && organism.health > 0.6 && organism.energy > 0.55) {
            organism.fertility += 0.08;
        } else {
            organism.fertility -= 0.05;
        }

        if (organism.reproduction_cooldown > 0) {
            organism.reproduction_cooldown -= 1;
        }

        organism.hunger = clamp(organism.hunger, 0.0, 2.0);
        organism.thirst = clamp(organism.thirst, 0.0, 2.0);
        organism.energy = clamp(organism.energy, 0.0, 1.0);
        organism.health = clamp(organism.health, 0.0, 1.0);
        organism.fertility = clamp(organism.fertility, 0.0, 1.0);

        if (organism.health <= 0.0) {
            organism.alive = false;
        }
    }

private:
    AgingModel aging_model_;

    static double clamp(double value, double low, double high) {
        return std::max(low, std::min(high, value));
    }

    static double compute_climate_stress(double temperature, std::pair<double, double> ideal_range) {
        auto [low, high] = ideal_range;
        if (low <= temperature && temperature <= high) return 0.0;
        if (temperature < low) return std::min(1.0, (low - temperature) / 20.0);
        return std::min(1.0, (temperature - high) / 20.0);
    }
};

} // namespace seeds::life
