#pragma once
#include <optional>
#include <string>

#include "../export.hpp"
#include "../core/engine.hpp"
#include "../core/time.hpp"
#include "../life/species.hpp"
#include "../world/map.hpp"
#include "schemas.hpp"

namespace seeds::api {

class SEEDS_API SeedsEngineFacade {
public:
    explicit SeedsEngineFacade(core::SeedsEngine engine);

    static SeedsEngineFacade create_phase1_ecology(
        int seed = 42,
        std::optional<core::TimeConfig> time_config = std::nullopt,
        std::optional<WorldConfig> world_config = std::nullopt
    );

    life::SpeciesProfile& register_species(life::SpeciesProfile profile);
    life::Organism& spawn(const SpawnRequest& request);
    std::vector<ecology::PopulationStats> step(int ticks = 1);
    storage::Snapshot snapshot();
    ai::AnalyticsReport analytics_report();

    core::SeedsEngine& engine();
    const core::SeedsEngine& engine() const;

private:
    core::SeedsEngine engine_;
};

SEEDS_API SeedsEngineFacade create_phase1_engine(
    int seed = 42,
    std::optional<core::TimeConfig> time_config = std::nullopt,
    std::optional<WorldConfig> world_config = std::nullopt
);

} // namespace seeds::api
