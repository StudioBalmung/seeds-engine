#pragma once
#include <map>
#include <string>
#include <stdexcept>

namespace seeds::world {

enum class BiomeType {
    Grassland,
    Forest,
    Desert,
    Wetland,
    Tundra,
};

inline std::string biome_to_string(BiomeType b) {
    switch (b) {
        case BiomeType::Grassland: return "grassland";
        case BiomeType::Forest:    return "forest";
        case BiomeType::Desert:    return "desert";
        case BiomeType::Wetland:   return "wetland";
        case BiomeType::Tundra:    return "tundra";
    }
    return "unknown";
}

inline BiomeType biome_from_string(const std::string& s) {
    if (s == "grassland") return BiomeType::Grassland;
    if (s == "forest")    return BiomeType::Forest;
    if (s == "desert")    return BiomeType::Desert;
    if (s == "wetland")   return BiomeType::Wetland;
    if (s == "tundra")    return BiomeType::Tundra;
    throw std::invalid_argument("Unknown biome: " + s);
}

struct BiomeProfile {
    BiomeType biome_type;
    double fertility_factor;
    double base_temperature;
    double base_humidity;
    double base_rainfall;
    double plant_growth_factor;
    int carrying_capacity;
    double water_retention;
    double default_water_level;
};

inline const std::map<BiomeType, BiomeProfile>& biome_library() {
    static const std::map<BiomeType, BiomeProfile> lib = {
        {BiomeType::Grassland, {BiomeType::Grassland, 1.0,  22.0, 0.45, 0.35, 1.0,  12, 0.7, 40.0}},
        {BiomeType::Forest,    {BiomeType::Forest,    1.2,  19.0, 0.65, 0.55, 1.3,  16, 0.9, 55.0}},
        {BiomeType::Desert,    {BiomeType::Desert,    0.35, 31.0, 0.18, 0.08, 0.25,  4, 0.2, 12.0}},
        {BiomeType::Wetland,   {BiomeType::Wetland,   1.1,  24.0, 0.75, 0.7,  1.25, 14, 1.0, 75.0}},
        {BiomeType::Tundra,    {BiomeType::Tundra,    0.45, -4.0, 0.4,  0.18, 0.2,   5, 0.5, 20.0}},
    };
    return lib;
}

inline const BiomeProfile& resolve_biome(BiomeType type) {
    return biome_library().at(type);
}

} // namespace seeds::world
