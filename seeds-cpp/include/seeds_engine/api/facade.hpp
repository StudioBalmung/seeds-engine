#pragma once
#include <optional>
#include <string>

#include "../core/engine.hpp"
#include "../core/time.hpp"
#include "../life/species.hpp"
#include "../world/map.hpp"
#include "schemas.hpp"

namespace seeds::api {

class SeedsEngineFacade {
public:
    explicit SeedsEngineFacade(core::SeedsEngine engine)
        : engine_(std::move(engine)) {}

    static SeedsEngineFacade create_phase1_ecology(
        int seed = 42,
        std::optional<core::TimeConfig> time_config = std::nullopt,
        std::optional<WorldConfig> world_config = std::nullopt
    ) {
        WorldConfig config = world_config.value_or(WorldConfig{});
        world::WorldMap world_map(config.width, config.height, config.chunk_size, config.default_biome);
        core::SeedsEngine engine(std::move(world_map), time_config.value_or(core::TimeConfig{}), seed);
        engine.register_species(life::SpeciesProfile::herbivore("herbivore", "Herbivore"));
        engine.register_species(life::SpeciesProfile::carnivore("carnivore", "Carnivore"));
        return SeedsEngineFacade(std::move(engine));
    }

    life::SpeciesProfile& register_species(life::SpeciesProfile profile) {
        return engine_.register_species(std::move(profile));
    }

    life::Organism& spawn(const SpawnRequest& request) {
        return engine_.spawn_organism(
            request.species_id,
            {request.x, request.y},
            request.sex.value_or(""),
            nullptr,
            request.age_ticks
        );
    }

    std::vector<ecology::PopulationStats> step(int ticks = 1) {
        return engine_.step(ticks);
    }

    storage::Snapshot snapshot() {
        return engine_.snapshot();
    }

    ai::AnalyticsReport analytics_report() {
        return engine_.analytics.summary(engine_);
    }

    core::SeedsEngine& engine() { return engine_; }
    const core::SeedsEngine& engine() const { return engine_; }

private:
    core::SeedsEngine engine_;
};

inline SeedsEngineFacade create_phase1_engine(
    int seed = 42,
    std::optional<core::TimeConfig> time_config = std::nullopt,
    std::optional<WorldConfig> world_config = std::nullopt
) {
    return SeedsEngineFacade::create_phase1_ecology(seed, time_config, world_config);
}

} // namespace seeds::api
