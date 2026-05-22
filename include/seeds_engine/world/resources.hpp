#pragma once
#include <algorithm>
#include <string>

#include "biome.hpp"
#include "climate.hpp"

namespace seeds::world {

struct ResourcePool {
    double plants = 40.0;
    double water = 40.0;
    double nutrients = 30.0;
    double max_plants = 100.0;
    double max_water = 100.0;
    double max_nutrients = 100.0;

    double consume_plants(double amount) {
        double consumed = std::min(plants, amount);
        plants -= consumed;
        nutrients = clamp(nutrients - consumed * 0.05, 0.0, max_nutrients);
        return consumed;
    }

    double consume_water(double amount) {
        double consumed = std::min(water, amount);
        water -= consumed;
        return consumed;
    }

    void regenerate(const BiomeProfile& biome, const ClimateState& climate,
                    double soil_fertility, bool water_body) {
        double plant_regen = soil_fertility * biome.fertility_factor * biome.plant_growth_factor * 1.8;
        if (climate.temperature < 0.0) {
            plant_regen *= 0.2;
        } else if (climate.temperature > 34.0) {
            plant_regen *= 0.55;
        }
        plant_regen *= 0.5 + climate.rainfall;

        double water_regen = climate.rainfall * (water_body ? 10.0 : 4.0);
        double evaporation = (climate.temperature >= 30.0) ? 0.8 : 0.35;

        double nutrient_regen = soil_fertility * 0.35 + (climate.season == "autumn" ? 0.2 : 0.0);

        plants = clamp(plants + plant_regen, 0.0, max_plants);
        water = clamp(water + water_regen - evaporation, 0.0, max_water);
        nutrients = clamp(nutrients + nutrient_regen, 0.0, max_nutrients);
    }

private:
    static double clamp(double value, double low, double high) {
        return std::max(low, std::min(high, value));
    }
};

} // namespace seeds::world
