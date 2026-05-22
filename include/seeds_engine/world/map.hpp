#pragma once
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "biome.hpp"
#include "climate.hpp"
#include "resources.hpp"
#include "terrain.hpp"

namespace seeds::world {

struct WorldTile {
    int x = 0;
    int y = 0;
    BiomeProfile biome;
    TerrainType terrain = TerrainType::Plain;
    double soil_fertility = 1.0;
    bool water_body = false;
    ResourcePool resources;
    ClimateState climate;
    std::set<std::string> occupants;

    [[nodiscard]] int carrying_capacity() const {
        const auto& terrain_profile = resolve_terrain(terrain);
        double modifier = (terrain_profile.terrain_type == TerrainType::Plain ||
                           terrain_profile.terrain_type == TerrainType::River)
                              ? 1.0
                              : 0.8;
        return std::max(1, static_cast<int>(biome.carrying_capacity * modifier));
    }
};

class WorldMap {
public:
    WorldMap(int width, int height, int chunk_size = 8, BiomeType default_biome = BiomeType::Grassland)
        : width_(width), height_(height), chunk_size_(chunk_size) {
        const auto& biome = resolve_biome(default_biome);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                bool wb = biome.default_water_level > 60.0;
                ResourcePool pool{
                    .plants = biome.carrying_capacity * 4.0,
                    .water = biome.default_water_level,
                    .nutrients = biome.fertility_factor * 30.0,
                    .max_plants = biome.carrying_capacity * 8.0,
                    .max_water = std::max(25.0, biome.default_water_level * 1.5),
                    .max_nutrients = 100.0,
                };
                WorldTile tile{
                    .x = x,
                    .y = y,
                    .biome = biome,
                    .terrain = TerrainType::Plain,
                    .soil_fertility = biome.fertility_factor,
                    .water_body = wb,
                    .resources = pool,
                    .climate = {},
                    .occupants = {},
                };
                tiles_[{x, y}] = std::move(tile);
            }
        }
    }

    [[nodiscard]] bool in_bounds(int x, int y) const noexcept {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    [[nodiscard]] WorldTile& get_tile(int x, int y) {
        return tiles_.at({x, y});
    }

    [[nodiscard]] const WorldTile& get_tile(int x, int y) const {
        return tiles_.at({x, y});
    }

    template <typename Fn>
    void for_each_tile(Fn&& fn) {
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                fn(tiles_[{x, y}]);
            }
        }
    }

    [[nodiscard]] std::vector<WorldTile*> neighbors(int x, int y) {
        std::vector<WorldTile*> result;
        constexpr int dirs[][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (auto [dx, dy] : dirs) {
            int nx = x + dx;
            int ny = y + dy;
            if (in_bounds(nx, ny)) {
                result.push_back(&tiles_[{nx, ny}]);
            }
        }
        return result;
    }

    [[nodiscard]] std::pair<int, int> chunk_for(int x, int y) const noexcept {
        return {x / chunk_size_, y / chunk_size_};
    }

    void place_organism(const std::string& organism_id, std::pair<int, int> position) {
        tiles_[position].occupants.insert(organism_id);
    }

    void remove_organism(const std::string& organism_id, std::pair<int, int> position) {
        tiles_[position].occupants.erase(organism_id);
    }

    void move_organism(const std::string& organism_id, std::pair<int, int> current, std::pair<int, int> destination) {
        if (current == destination) return;
        remove_organism(organism_id, current);
        place_organism(organism_id, destination);
    }

    void set_biome(int x, int y, BiomeType biome_type) {
        auto& tile = get_tile(x, y);
        const auto& biome = resolve_biome(biome_type);
        tile.biome = biome;
        tile.soil_fertility = biome.fertility_factor;
        tile.water_body = biome.default_water_level > 60.0;
        tile.resources.max_plants = biome.carrying_capacity * 8.0;
        tile.resources.max_water = std::max(25.0, biome.default_water_level * 1.5);
    }

    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }

private:
    int width_;
    int height_;
    int chunk_size_;
    std::map<std::pair<int, int>, WorldTile> tiles_;
};

} // namespace seeds::world
