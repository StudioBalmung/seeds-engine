#include <iostream>
#include <string>

#include <seeds_engine/seeds_engine.hpp>

using namespace seeds;

int main(int argc, char* argv[]) {
    int ticks = 24;
    int seed = 42;
    int tick_minutes = 60;
    int width = 8;
    int height = 8;
    int chunk_size = 4;
    int herbivores = 4;
    int carnivores = 2;

    // Simple arg parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--ticks" && i + 1 < argc) ticks = std::stoi(argv[++i]);
        else if (arg == "--seed" && i + 1 < argc) seed = std::stoi(argv[++i]);
        else if (arg == "--tick-minutes" && i + 1 < argc) tick_minutes = std::stoi(argv[++i]);
        else if (arg == "--width" && i + 1 < argc) width = std::stoi(argv[++i]);
        else if (arg == "--height" && i + 1 < argc) height = std::stoi(argv[++i]);
        else if (arg == "--chunk-size" && i + 1 < argc) chunk_size = std::stoi(argv[++i]);
        else if (arg == "--herbivores" && i + 1 < argc) herbivores = std::stoi(argv[++i]);
        else if (arg == "--carnivores" && i + 1 < argc) carnivores = std::stoi(argv[++i]);
    }

    core::TimeConfig time_config;
    time_config.tick_minutes = tick_minutes;

    api::WorldConfig world_config;
    world_config.width = width;
    world_config.height = height;
    world_config.chunk_size = chunk_size;
    world_config.default_biome = world::BiomeType::Grassland;

    auto facade = api::create_phase1_engine(seed, time_config, world_config);

    // Spawn herbivores
    for (int i = 0; i < herbivores; ++i) {
        int x = (i * 2) % width;
        int y = ((i * 2) / width) % height;
        std::string sex = (i % 2 == 0) ? "female" : "male";
        facade.spawn(api::SpawnRequest{.species_id = "herbivore", .x = x, .y = y, .sex = sex, .age_ticks = 24});
    }

    // Spawn carnivores
    for (int i = 0; i < carnivores; ++i) {
        int x = (width - 1 - i) % width;
        int y = (height - 1 - (i / std::max(1, width))) % height;
        std::string sex = (i % 2 == 0) ? "female" : "male";
        facade.spawn(api::SpawnRequest{.species_id = "carnivore", .x = x, .y = y, .sex = sex, .age_ticks = 24});
    }

    facade.step(ticks);

    auto snap = facade.snapshot();
    auto report = facade.analytics_report();

    // Print summary
    std::cout << "=== SUMMARY ===\n";
    std::cout << "tick: " << std::get<int>(snap["tick"]) << "\n";
    std::cout << "season: " << std::get<std::string>(snap["season"]) << "\n";
    std::cout << "alive: " << std::get<int>(snap["total_alive"]) << "\n";

    auto& counts = std::get<std::map<std::string, int>>(snap["species_counts"]);
    std::cout << "species_counts: {";
    bool first = true;
    for (const auto& [k, v] : counts) {
        if (!first) std::cout << ", ";
        first = false;
        std::cout << k << ": " << v;
    }
    std::cout << "}\n";

    auto& resources = std::get<std::map<std::string, double>>(snap["world_resources"]);
    std::cout << "resources: {";
    first = true;
    for (const auto& [k, v] : resources) {
        if (!first) std::cout << ", ";
        first = false;
        std::cout << k << ": " << v;
    }
    std::cout << "}\n";

    auto& anomalies = std::get<std::vector<ai::AnomalyFinding>>(report["anomalies"]);
    std::cout << "anomalies: [";
    first = true;
    for (const auto& a : anomalies) {
        if (!first) std::cout << ", ";
        first = false;
        std::cout << "{" << a.severity << ": " << a.reason << "}";
    }
    std::cout << "]\n";

    return 0;
}
