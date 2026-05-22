#pragma once
#include <algorithm>
#include <map>
#include <string>

#include "biome.hpp"

namespace seeds::world {

struct ClimateState {
    double temperature = 20.0;
    double humidity = 0.5;
    double rainfall = 0.2;
    std::string season = "spring";
    bool is_daylight = true;
};

class ClimateEngine {
public:
    ClimateState update(const BiomeProfile& biome, const std::string& season,
                        bool is_daylight, double water_bonus = 0.0) const {
        double temperature = biome.base_temperature + season_temperature(season) + (is_daylight ? 2.0 : -2.0);
        double humidity = clamp(
            biome.base_humidity + season_rainfall(season) + water_bonus * 0.01, 0.0, 1.0);
        double rainfall = clamp(
            biome.base_rainfall + season_rainfall(season) + water_bonus * 0.002, 0.0, 1.0);
        return ClimateState{temperature, humidity, rainfall, season, is_daylight};
    }

private:
    static double clamp(double value, double low, double high) {
        return std::max(low, std::min(high, value));
    }

    static double season_temperature(const std::string& season) {
        if (season == "spring") return 0.0;
        if (season == "summer") return 6.0;
        if (season == "autumn") return -1.5;
        if (season == "winter") return -8.0;
        return 0.0;
    }

    static double season_rainfall(const std::string& season) {
        if (season == "spring") return 0.1;
        if (season == "summer") return 0.05;
        if (season == "autumn") return 0.0;
        if (season == "winter") return -0.08;
        return 0.0;
    }
};

} // namespace seeds::world
