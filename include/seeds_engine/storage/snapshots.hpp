#pragma once
#include <any>
#include <cmath>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace seeds::storage {

using SnapshotValue = std::variant<int, double, std::string, std::map<std::string, int>,
                                   std::map<std::string, double>, std::vector<std::map<std::string, std::string>>>;
using Snapshot = std::map<std::string, SnapshotValue>;

class SnapshotSerializer {
public:
    template <typename Engine>
    static Snapshot from_engine(Engine& engine) {
        double total_plants = 0.0, total_water = 0.0, total_nutrients = 0.0;
        engine.world.for_each_tile([&](auto& tile) {
            total_plants += tile.resources.plants;
            total_water += tile.resources.water;
            total_nutrients += tile.resources.nutrients;
        });

        std::map<std::string, double> world_resources;
        world_resources["plants"] = std::round(total_plants * 1000.0) / 1000.0;
        world_resources["water"] = std::round(total_water * 1000.0) / 1000.0;
        world_resources["nutrients"] = std::round(total_nutrients * 1000.0) / 1000.0;

        int total_alive = 0;
        std::vector<std::map<std::string, std::string>> organisms_data;
        for (const auto& [id, organism] : engine.organisms) {
            if (organism.alive) total_alive++;
            std::map<std::string, std::string> org_map;
            org_map["id"] = organism.organism_id;
            org_map["species"] = organism.species_id;
            org_map["position_x"] = std::to_string(organism.position.first);
            org_map["position_y"] = std::to_string(organism.position.second);
            org_map["alive"] = organism.alive ? "true" : "false";
            org_map["health"] = std::to_string(organism.health);
            org_map["energy"] = std::to_string(organism.energy);
            org_map["hunger"] = std::to_string(organism.hunger);
            org_map["thirst"] = std::to_string(organism.thirst);
            org_map["generation"] = std::to_string(organism.generation);
            for (const auto& [trait_name, trait_val] : organism.phenotype.traits) {
                org_map["trait_" + trait_name] = trait_val;
            }
            organisms_data.push_back(std::move(org_map));
        }

        std::map<std::string, int> species_counts;
        if (!engine.population_history.empty()) {
            species_counts = engine.population_history.back().species_counts;
        }

        Snapshot snapshot;
        snapshot["tick"] = engine.clock.tick();
        snapshot["season"] = engine.clock.season();
        snapshot["total_alive"] = total_alive;
        snapshot["species_counts"] = species_counts;
        snapshot["world_resources"] = world_resources;
        snapshot["organisms"] = organisms_data;
        return snapshot;
    }
};

} // namespace seeds::storage
