#pragma once
#include <algorithm>
#include <map>
#include <random>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "../export.hpp"
#include "events.hpp"
#include "scheduler.hpp"
#include "time.hpp"
#include "../ai/reports.hpp"
#include "../behavior/actions.hpp"
#include "../behavior/perception.hpp"
#include "../behavior/utility_ai.hpp"
#include "../debug/debugger.hpp"
#include "../ecology/competition.hpp"
#include "../ecology/foodweb.hpp"
#include "../ecology/migration.hpp"
#include "../ecology/population.hpp"
#include "../ecology/predation.hpp"
#include "../life/disease.hpp"
#include "../life/faith.hpp"
#include "../life/genetics.hpp"
#include "../life/organism.hpp"
#include "../life/physiology.hpp"
#include "../life/reproduction.hpp"
#include "../life/species.hpp"
#include "../storage/repository.hpp"
#include "../storage/snapshots.hpp"
#include "../world/climate.hpp"
#include "../world/map.hpp"

namespace seeds::core {

class SEEDS_API SeedsEngine {
public:
    SeedsEngine(world::WorldMap world_map, TimeConfig time_config = {}, int seed = 42);

    life::SpeciesProfile& register_species(life::SpeciesProfile profile);

    life::Organism& spawn_organism(
        const std::string& species_id,
        std::pair<int, int> position,
        const std::string& sex = "",
        life::Genome* genome_ptr = nullptr,
        int age_ticks = 0,
        int generation = 0,
        std::vector<std::string> parents = {},
        bool register_event = true
    );

    std::vector<ecology::PopulationStats> step(int ticks = 1);

    void schedule_disaster(int tick, std::pair<int, int> position, int radius, double severity);
    void mark_death(const std::string& organism_id, const std::string& reason);
    storage::Snapshot snapshot();

    // Faith module (v1.0.0)
    life::FaithEngine& faith_engine() noexcept { return faith_engine_; }
    const life::FaithEngine& faith_engine() const noexcept { return faith_engine_; }

    // Debugger (v1.0.0)
    debug::SeedsDebugger& debugger() noexcept { return debugger_; }
    const debug::SeedsDebugger& debugger() const noexcept { return debugger_; }

    // Public members for template access
    world::WorldMap world;
    SimulationClock clock;
    EventScheduler scheduler;
    std::mt19937 rng;
    std::map<std::string, life::SpeciesProfile> species;
    std::map<std::string, life::Organism> organisms;
    std::vector<EventRecord> event_log;
    std::vector<ecology::PopulationStats> population_history;
    ai::AnalyticsReportService analytics;

private:
    life::GeneticsEngine genetics_;
    life::FaithEngine faith_engine_;
    debug::SeedsDebugger debugger_;
    world::ClimateEngine climate_;
    life::PhysiologyEngine physiology_;
    behavior::UtilityBehavior behavior_;
    life::DiseaseEngine disease_engine_;
    ecology::FoodWeb foodweb_;
    ecology::CompetitionEngine competition_;
    ecology::MigrationPlanner migration_;
    ecology::PredationEngine predation_;
    life::ReproductionEngine reproduction_;
    ecology::PopulationTracker population_tracker_;
    storage::InMemorySnapshotRepository snapshot_repository_;
    int next_organism_id_ = 1;
    int births_this_tick_ = 0;
    int deaths_this_tick_ = 0;

    static std::string pad_id(int id);
    ecology::PopulationStats step_once();
    void update_world();
    void dispatch_due_events();
    void handle_birth(const SimulationEvent& event);
    void handle_disaster(const SimulationEvent& event);
    behavior::PerceptionSnapshot build_perception(const life::Organism& organism);
    void execute_action(
        life::Organism& organism,
        const life::SpeciesProfile& profile,
        world::WorldTile& tile,
        const behavior::ActionDecision& decision,
        const behavior::PerceptionSnapshot& perception,
        std::map<std::tuple<int, int, std::string>, std::map<std::string, std::vector<std::string>>>& mating_intents
    );
};

} // namespace seeds::core
