#pragma once
#include <string>
#include <utility>

namespace seeds::life {

enum class DietType {
    Herbivore,
    Carnivore,
    Omnivore,
};

inline std::string diet_to_string(DietType d) {
    switch (d) {
        case DietType::Herbivore: return "herbivore";
        case DietType::Carnivore: return "carnivore";
        case DietType::Omnivore:  return "omnivore";
    }
    return "unknown";
}

struct SpeciesProfile {
    std::string species_id;
    std::string display_name;
    DietType diet = DietType::Herbivore;
    int mature_age_ticks = 24;
    int max_age_ticks = 24 * 180;
    double metabolism = 0.06;
    double movement_cost = 0.08;
    double reproduction_energy_threshold = 0.65;
    double reproduction_health_threshold = 0.65;
    double reproduction_hunger_limit = 0.45;
    double reproduction_thirst_limit = 0.45;
    int gestation_ticks = 6;
    std::pair<int, int> litter_size_range = {1, 2};
    double base_body_mass = 10.0;
    double water_need = 0.05;
    double grazing_rate = 8.0;
    double predation_power = 0.0;
    int sensory_range = 1;
    int movement_range = 1;
    std::pair<double, double> ideal_temperature_range = {5.0, 28.0};

    [[nodiscard]] bool can_predate() const noexcept {
        return diet == DietType::Carnivore || diet == DietType::Omnivore;
    }

    static SpeciesProfile herbivore(const std::string& species_id, const std::string& display_name) {
        return SpeciesProfile{
            .species_id = species_id,
            .display_name = display_name,
            .diet = DietType::Herbivore,
            .mature_age_ticks = 18,
            .max_age_ticks = 24 * 120,
            .metabolism = 0.055,
            .movement_cost = 0.07,
            .reproduction_energy_threshold = 0.65,
            .reproduction_health_threshold = 0.65,
            .reproduction_hunger_limit = 0.45,
            .reproduction_thirst_limit = 0.45,
            .gestation_ticks = 5,
            .litter_size_range = {1, 3},
            .base_body_mass = 14.0,
            .water_need = 0.06,
            .grazing_rate = 10.0,
            .predation_power = 0.0,
            .sensory_range = 1,
            .movement_range = 1,
            .ideal_temperature_range = {2.0, 30.0},
        };
    }

    static SpeciesProfile carnivore(const std::string& species_id, const std::string& display_name) {
        return SpeciesProfile{
            .species_id = species_id,
            .display_name = display_name,
            .diet = DietType::Carnivore,
            .mature_age_ticks = 24,
            .max_age_ticks = 24 * 160,
            .metabolism = 0.065,
            .movement_cost = 0.09,
            .reproduction_energy_threshold = 0.65,
            .reproduction_health_threshold = 0.65,
            .reproduction_hunger_limit = 0.45,
            .reproduction_thirst_limit = 0.45,
            .gestation_ticks = 7,
            .litter_size_range = {1, 2},
            .base_body_mass = 22.0,
            .water_need = 0.07,
            .grazing_rate = 0.0,
            .predation_power = 0.75,
            .sensory_range = 2,
            .movement_range = 1,
            .ideal_temperature_range = {0.0, 27.0},
        };
    }
};

} // namespace seeds::life
