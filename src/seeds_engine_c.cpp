#include <seeds_engine/seeds_engine_c.h>
#include <seeds_engine/seeds_engine.hpp>
#include <seeds_engine/life/faith.hpp>
#include <seeds_engine/debug/debugger.hpp>

#include <cstring>
#include <map>
#include <sstream>
#include <string>

// Internal wrapper holding C++ objects behind the opaque handle
struct SeedsEngine_t {
    seeds::core::SeedsEngine engine;
    seeds::life::FaithEngine faith_engine;
    seeds::debug::SeedsDebugger debugger{50000};
    std::map<std::string, seeds::life::FaithState> faith_states;

    // Scratch buffers for returning strings to C callers
    std::string scratch_str;
    std::string scratch_json;
    std::string last_spawned_id;

    SeedsEngine_t(seeds::world::WorldMap world_map, seeds::core::TimeConfig tc, int seed)
        : engine(std::move(world_map), tc, seed) {
        faith_engine.register_defaults();
    }
};

static char* alloc_cstr(const std::string& s) {
    char* buf = static_cast<char*>(std::malloc(s.size() + 1));
    if (buf) {
        std::memcpy(buf, s.c_str(), s.size() + 1);
    }
    return buf;
}

// ================================================================
// Lifecycle
// ================================================================

extern "C" {

SEEDS_CAPI SeedsEngineHandle seeds_create(int seed, int width, int height, int tick_minutes) {
    try {
        seeds::core::TimeConfig tc;
        tc.tick_minutes = tick_minutes;
        seeds::world::WorldMap world_map(width, height, 4, seeds::world::BiomeType::Grassland);
        auto* handle = new SeedsEngine_t(std::move(world_map), tc, seed);
        handle->engine.register_species(
            seeds::life::SpeciesProfile::herbivore("herbivore", "Herbivore"));
        handle->engine.register_species(
            seeds::life::SpeciesProfile::carnivore("carnivore", "Carnivore"));
        return handle;
    } catch (...) {
        return nullptr;
    }
}

SEEDS_CAPI void seeds_destroy(SeedsEngineHandle handle) {
    delete handle;
}

// ================================================================
// Species Registration
// ================================================================

SEEDS_CAPI void seeds_register_herbivore(SeedsEngineHandle handle, const char* species_id, const char* name) {
    if (!handle) return;
    handle->engine.register_species(
        seeds::life::SpeciesProfile::herbivore(species_id, name));
}

SEEDS_CAPI void seeds_register_carnivore(SeedsEngineHandle handle, const char* species_id, const char* name) {
    if (!handle) return;
    handle->engine.register_species(
        seeds::life::SpeciesProfile::carnivore(species_id, name));
}

// ================================================================
// Spawning
// ================================================================

SEEDS_CAPI const char* seeds_spawn(SeedsEngineHandle handle,
                                   const char* species_id,
                                   int x, int y,
                                   const char* sex,
                                   int age_ticks) {
    if (!handle) return "";
    try {
        auto& org = handle->engine.spawn_organism(
            species_id, {x, y}, sex ? sex : "", nullptr, age_ticks);
        handle->last_spawned_id = org.organism_id;
        return handle->last_spawned_id.c_str();
    } catch (...) {
        return "";
    }
}

// ================================================================
// Simulation Control
// ================================================================

SEEDS_CAPI void seeds_step(SeedsEngineHandle handle, int ticks) {
    if (!handle || ticks < 1) return;
    handle->engine.step(ticks);
}

SEEDS_CAPI void seeds_schedule_disaster(SeedsEngineHandle handle,
                                        int tick, int x, int y,
                                        int radius, double severity) {
    if (!handle) return;
    handle->engine.schedule_disaster(tick, {x, y}, radius, severity);
}

// ================================================================
// Queries
// ================================================================

SEEDS_CAPI int seeds_get_tick(SeedsEngineHandle handle) {
    if (!handle) return 0;
    return handle->engine.clock.tick();
}

SEEDS_CAPI const char* seeds_get_season(SeedsEngineHandle handle) {
    if (!handle) return "";
    handle->scratch_str = handle->engine.clock.season();
    return handle->scratch_str.c_str();
}

SEEDS_CAPI int seeds_get_alive_count(SeedsEngineHandle handle) {
    if (!handle) return 0;
    int count = 0;
    for (const auto& [id, org] : handle->engine.organisms)
        if (org.alive) count++;
    return count;
}

SEEDS_CAPI int seeds_get_species_count(SeedsEngineHandle handle, const char* species_id) {
    if (!handle || !species_id) return 0;
    int count = 0;
    for (const auto& [id, org] : handle->engine.organisms)
        if (org.alive && org.species_id == species_id) count++;
    return count;
}

SEEDS_CAPI char* seeds_snapshot_json(SeedsEngineHandle handle) {
    if (!handle) return alloc_cstr("{}");
    auto snap = handle->engine.snapshot();
    std::ostringstream oss;
    oss << "{\"tick\":" << std::get<int>(snap["tick"])
        << ",\"season\":\"" << std::get<std::string>(snap["season"]) << "\""
        << ",\"total_alive\":" << std::get<int>(snap["total_alive"]);
    auto& counts = std::get<std::map<std::string, int>>(snap["species_counts"]);
    oss << ",\"species_counts\":{";
    bool first = true;
    for (const auto& [k, v] : counts) {
        if (!first) oss << ",";
        first = false;
        oss << "\"" << k << "\":" << v;
    }
    oss << "}";
    auto& res = std::get<std::map<std::string, double>>(snap["world_resources"]);
    oss << ",\"world_resources\":{";
    first = true;
    for (const auto& [k, v] : res) {
        if (!first) oss << ",";
        first = false;
        oss << "\"" << k << "\":" << v;
    }
    oss << "}}";
    return alloc_cstr(oss.str());
}

SEEDS_CAPI char* seeds_analytics_json(SeedsEngineHandle handle) {
    if (!handle) return alloc_cstr("{}");
    auto report = handle->engine.analytics.summary(handle->engine);
    std::ostringstream oss;
    oss << "{\"tick\":" << std::get<int>(report["tick"])
        << ",\"season\":\"" << std::get<std::string>(report["season"]) << "\"";
    auto& cap = std::get<std::map<std::string, double>>(report["carrying_capacity"]);
    oss << ",\"carrying_capacity\":{";
    bool first = true;
    for (const auto& [k, v] : cap) {
        if (!first) oss << ",";
        first = false;
        oss << "\"" << k << "\":" << v;
    }
    oss << "}";
    auto& anomalies = std::get<std::vector<seeds::ai::AnomalyFinding>>(report["anomalies"]);
    oss << ",\"anomalies\":[";
    first = true;
    for (const auto& a : anomalies) {
        if (!first) oss << ",";
        first = false;
        oss << "{\"severity\":\"" << a.severity << "\",\"reason\":\"" << a.reason << "\"}";
    }
    oss << "]}";
    return alloc_cstr(oss.str());
}

// ================================================================
// Organism Query
// ================================================================

SEEDS_CAPI double seeds_organism_health(SeedsEngineHandle handle, const char* organism_id) {
    if (!handle || !organism_id) return -1.0;
    auto it = handle->engine.organisms.find(organism_id);
    if (it == handle->engine.organisms.end()) return -1.0;
    return it->second.health;
}

SEEDS_CAPI double seeds_organism_energy(SeedsEngineHandle handle, const char* organism_id) {
    if (!handle || !organism_id) return -1.0;
    auto it = handle->engine.organisms.find(organism_id);
    if (it == handle->engine.organisms.end()) return -1.0;
    return it->second.energy;
}

SEEDS_CAPI int seeds_organism_alive(SeedsEngineHandle handle, const char* organism_id) {
    if (!handle || !organism_id) return -1;
    auto it = handle->engine.organisms.find(organism_id);
    if (it == handle->engine.organisms.end()) return -1;
    return it->second.alive ? 1 : 0;
}

SEEDS_CAPI int seeds_organism_position(SeedsEngineHandle handle, const char* organism_id,
                                       int* out_x, int* out_y) {
    if (!handle || !organism_id || !out_x || !out_y) return -1;
    auto it = handle->engine.organisms.find(organism_id);
    if (it == handle->engine.organisms.end()) return -1;
    *out_x = it->second.position.first;
    *out_y = it->second.position.second;
    return 0;
}

// ================================================================
// Faith Module
// ================================================================

SEEDS_CAPI void seeds_set_belief(SeedsEngineHandle handle,
                                 const char* organism_id,
                                 const char* belief_id,
                                 double devotion) {
    if (!handle || !organism_id || !belief_id) return;
    auto& fs = handle->faith_states[organism_id];
    handle->faith_engine.assign_belief(fs, belief_id, devotion);
}

SEEDS_CAPI const char* seeds_get_belief(SeedsEngineHandle handle, const char* organism_id) {
    if (!handle || !organism_id) return "";
    auto it = handle->faith_states.find(organism_id);
    if (it == handle->faith_states.end()) return "";
    handle->scratch_str = it->second.belief_id;
    return handle->scratch_str.c_str();
}

SEEDS_CAPI double seeds_get_devotion(SeedsEngineHandle handle, const char* organism_id) {
    if (!handle || !organism_id) return -1.0;
    auto it = handle->faith_states.find(organism_id);
    if (it == handle->faith_states.end()) return -1.0;
    return it->second.devotion;
}

// ================================================================
// Debugger
// ================================================================

SEEDS_CAPI void seeds_debug_enable(SeedsEngineHandle handle) {
    if (!handle) return;
    handle->debugger.enable();
}

SEEDS_CAPI void seeds_debug_disable(SeedsEngineHandle handle) {
    if (!handle) return;
    handle->debugger.disable();
}

SEEDS_CAPI char* seeds_debug_flush(SeedsEngineHandle handle) {
    if (!handle) return alloc_cstr("[]");
    std::string json = handle->debugger.to_json();
    handle->debugger.flush();
    return alloc_cstr(json);
}

// ================================================================
// Memory Management
// ================================================================

SEEDS_CAPI void seeds_free_string(char* str) {
    std::free(str);
}

// ================================================================
// Version
// ================================================================

SEEDS_CAPI const char* seeds_version(void) {
    return "1.0.0";
}

} // extern "C"
