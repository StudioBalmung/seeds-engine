#pragma once
#include <map>
#include <string>
#include <stdexcept>

namespace seeds::world {

enum class TerrainType {
    Plain,
    Hill,
    Mountain,
    River,
    Lake,
};

inline std::string terrain_to_string(TerrainType t) {
    switch (t) {
        case TerrainType::Plain:    return "plain";
        case TerrainType::Hill:     return "hill";
        case TerrainType::Mountain: return "mountain";
        case TerrainType::River:    return "river";
        case TerrainType::Lake:     return "lake";
    }
    return "unknown";
}

struct TerrainProfile {
    TerrainType terrain_type;
    double movement_cost;
    double shelter_factor;
    double water_bonus;
};

inline const std::map<TerrainType, TerrainProfile>& terrain_library() {
    static const std::map<TerrainType, TerrainProfile> lib = {
        {TerrainType::Plain,    {TerrainType::Plain,    1.0, 0.1,  0.0}},
        {TerrainType::Hill,     {TerrainType::Hill,     1.2, 0.2,  0.0}},
        {TerrainType::Mountain, {TerrainType::Mountain, 1.5, 0.35, 0.0}},
        {TerrainType::River,    {TerrainType::River,    1.1, 0.0, 18.0}},
        {TerrainType::Lake,     {TerrainType::Lake,     1.4, 0.0, 32.0}},
    };
    return lib;
}

inline const TerrainProfile& resolve_terrain(TerrainType type) {
    return terrain_library().at(type);
}

} // namespace seeds::world
