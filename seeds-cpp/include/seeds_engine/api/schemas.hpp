#pragma once
#include <optional>
#include <string>

#include "../world/biome.hpp"

namespace seeds::api {

struct WorldConfig {
    int width = 8;
    int height = 8;
    int chunk_size = 4;
    world::BiomeType default_biome = world::BiomeType::Grassland;
};

struct SpawnRequest {
    std::string species_id;
    int x = 0;
    int y = 0;
    std::optional<std::string> sex = std::nullopt;
    int age_ticks = 0;
};

} // namespace seeds::api
