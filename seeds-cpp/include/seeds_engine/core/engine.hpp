#pragma once
#include <algorithm>
#include <map>
#include <random>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "events.hpp"
#include "scheduler.hpp"
#include "time.hpp"
#include "../ai/reports.hpp"
#include "../behavior/actions.hpp"
#include "../behavior/perception.hpp"
#include "../behavior/utility_ai.hpp"
#include "../ecology/competition.hpp"
#include "../ecology/foodweb.hpp"
#include "../ecology/migration.hpp"
#include "../ecology/population.hpp"
#include "../ecology/predation.hpp"
#include "../life/disease.hpp"
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

class SeedsEngine {
public:
    SeedsEngine(world::WorldMap world_map, TimeConfig time_config = {}, int seed = 42)
        : world(std::move(world_map)),
          clock(time_config),
          rng(static_cast<unsigned>(seed)),
          genetics_(rng),
          migration_(rng),
          reproduction_(rng) {}

    life::SpeciesProfile& register_species(life::SpeciesProfile profile) {
        std::string id = profile.species_id;
        species[id] = std::move(profile);
        return species[id];
    }

    life::Organism& spawn_organism(
        const std::string& species_id,
        std::pair<int, int> position,
        const std::string& sex = "",
        life::Genome* genome_ptr = nullptr,
        int age_ticks = 0,
        int generation = 0,
        std::vector<std::string> parents = {},
        bool register_event = true
    ) {
        const auto& profile = species.at(species_id);
        std::string actual_sex = sex;
        if (actual_sex.empty()) {
            std::uniform_int_distribution<int> dist(0, 1);
            actual_sex = dist(rng) ? "male" : "female";
        }
        life::Genome actual_genome = genome_ptr ? *genome_ptr : genetics_.create_founder();
        life::Phenotype phenotype = genetics_.express(actual_genome);

        std::string organism_id = "O" + pad_id(next_organism_id_++);

        life::Organism organism;
        organism.organism_id = organism_id;
        organism.species_id = species_id;
        organism.sex = actual_sex;
        organism.position = position;
        organism.genome = std::move(actual_genome);
        organism.phenotype = std::move(phenotype);
        organism.age_ticks = age_ticks;
        organism.body_mass = std::max(1.0, profile.base_body_mass * (age_ticks == 0 ? 0.3 : 1.0));
        organism.lifespan_ticks = profile.max_age_ticks;
        organism.generation = generation;
        organism.parents = std::move(parents);
        organism.sensory_capability = static_cast<double>(profile.sensory_range);
        organism.movement_capability = profile.movement_range;
        organism.fertility = (age_ticks >= profile.mature_age_ticks) ? 0.8 : 0.0;

        organisms[organism_id] = std::move(organism);
        world.place_organism(organism_id, position);

        if (register_event) {
            event_log.push_back(EventRecord{
                .tick = clock.tick(),
                .event_type = EventType::WorldEvent,
                .details = {{"event", "spawn"}, {"organism_id", organism_id}, {"species_id", species_id}},
            });
        }
        return organisms[organism_id];
    }

    std::vector<ecology::PopulationStats> step(int ticks = 1) {
        std::vector<ecology::PopulationStats> results;
        for (int i = 0; i < ticks; ++i) {
            results.push_back(step_once());
        }
        return results;
    }

    void schedule_disaster(int tick, std::pair<int, int> position, int radius, double severity) {
        SimulationEvent event;
        event.tick = tick;
        event.priority = 5;
        event.event_type = EventType::Disaster;
        event.payload["position_x"] = std::to_string(position.first);
        event.payload["position_y"] = std::to_string(position.second);
        event.payload["radius"] = std::to_string(radius);
        event.payload["severity"] = std::to_string(severity);
        scheduler.schedule(std::move(event));
    }

    void mark_death(const std::string& organism_id, const std::string& reason) {
        auto& organism = organisms[organism_id];
        if (!organism.alive && organism.last_action.starts_with("dead:")) return;
        organism.alive = false;
        organism.behavior_state = "dead";
        organism.last_action = "dead:" + reason;
        world.remove_organism(organism.organism_id, organism.position);
        deaths_this_tick_++;
        event_log.push_back(EventRecord{
            .tick = clock.tick(),
            .event_type = EventType::OrganismDeath,
            .details = {{"organism_id", organism.organism_id}, {"reason", reason}},
        });
    }

    storage::Snapshot snapshot() {
        return storage::SnapshotSerializer::from_engine(*this);
    }

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

    static std::string pad_id(int id) {
        std::string s = std::to_string(id);
        while (s.size() < 6) s = "0" + s;
        return s;
    }

    ecology::PopulationStats step_once() {
        clock.step();
        births_this_tick_ = 0;
        deaths_this_tick_ = 0;
        update_world();
        dispatch_due_events();

        using MatingKey = std::tuple<int, int, std::string>;
        std::map<MatingKey, std::map<std::string, std::vector<std::string>>> mating_intents;

        std::vector<std::string> organism_ids;
        for (const auto& [id, _] : organisms) organism_ids.push_back(id);
        std::sort(organism_ids.begin(), organism_ids.end());

        for (const auto& organism_id : organism_ids) {
            auto& organism = organisms[organism_id];
            if (!organism.alive) continue;
            const auto& profile = species.at(organism.species_id);
            auto& tile = world.get_tile(organism.position.first, organism.position.second);

            physiology_.apply_tick(organism, profile, tile);
            if (!organism.alive || organism.health <= 0.0) {
                mark_death(organism.organism_id, "physiology_failure");
                continue;
            }

            int density = static_cast<int>(tile.occupants.size());
            disease_engine_.update(organism, density, rng);
            if (organism.health <= 0.0) {
                mark_death(organism.organism_id, "disease");
                continue;
            }

            auto perception = build_perception(organism);
            auto decision = behavior_.select_action(organism, profile, perception);
            execute_action(organism, profile, tile, decision, perception, mating_intents);
            if (organism.health <= 0.0) {
                mark_death(organism.organism_id, "action_cost");
            }
        }

        reproduction_.queue_births(*this, mating_intents);

        auto stats = population_tracker_.summarize(organisms, clock.tick(), births_this_tick_, deaths_this_tick_);
        population_history.push_back(stats);
        snapshot_repository_.save(snapshot());
        return stats;
    }

    void update_world() {
        world.for_each_tile([this](world::WorldTile& tile) {
            tile.climate = climate_.update(tile.biome, clock.season(), clock.is_daylight(), tile.resources.water);
            tile.resources.regenerate(tile.biome, tile.climate, tile.soil_fertility, tile.water_body);
        });
    }

    void dispatch_due_events() {
        for (auto& event : scheduler.pop_due(clock.tick())) {
            if (event.event_type == EventType::OrganismBirth) {
                handle_birth(event);
            } else if (event.event_type == EventType::Disaster) {
                handle_disaster(event);
            }
        }
    }

    void handle_birth(const SimulationEvent& event) {
        const std::string& mother_id = event.payload.at("mother_id");
        const std::string& father_id = event.payload.at("father_id");
        if (!organisms.count(mother_id) || !organisms.count(father_id)) return;
        auto& mother = organisms[mother_id];
        auto& father = organisms[father_id];
        if (!mother.alive || !father.alive) return;

        life::Genome child_genome = genetics_.crossover(mother.genome, father.genome);
        int px = std::stoi(event.payload.at("position_x"));
        int py = std::stoi(event.payload.at("position_y"));
        int gen = std::max(mother.generation, father.generation) + 1;

        auto& child = spawn_organism(
            event.payload.at("species_id"),
            {px, py},
            "",
            &child_genome,
            0,
            gen,
            {mother.organism_id, father.organism_id},
            false
        );
        child.energy = 0.85;
        child.health = 1.0;
        child.hunger = 0.15;
        child.thirst = 0.15;
        mother.offspring_count++;
        father.offspring_count++;
        births_this_tick_++;
        event_log.push_back(EventRecord{
            .tick = clock.tick(),
            .event_type = EventType::OrganismBirth,
            .details = {{"child_id", child.organism_id}, {"mother_id", mother.organism_id}, {"father_id", father.organism_id}},
        });
    }

    void handle_disaster(const SimulationEvent& event) {
        int cx = std::stoi(event.payload.at("position_x"));
        int cy = std::stoi(event.payload.at("position_y"));
        int radius = std::stoi(event.payload.at("radius"));
        double severity = std::stod(event.payload.at("severity"));

        world.for_each_tile([&](world::WorldTile& tile) {
            if (std::abs(tile.x - cx) + std::abs(tile.y - cy) > radius) return;
            tile.resources.plants = std::max(0.0, tile.resources.plants * (1.0 - severity));
            tile.resources.water = std::max(0.0, tile.resources.water * (1.0 - severity * 0.6));
        });
        event_log.push_back(EventRecord{
            .tick = clock.tick(),
            .event_type = EventType::Disaster,
            .details = {{"position_x", std::to_string(cx)}, {"position_y", std::to_string(cy)},
                        {"radius", std::to_string(radius)}, {"severity", std::to_string(severity)}},
        });
    }

    behavior::PerceptionSnapshot build_perception(const life::Organism& organism) {
        auto& tile = world.get_tile(organism.position.first, organism.position.second);
        auto neighbor_ptrs = world.neighbors(organism.position.first, organism.position.second);
        const auto& species_profile = species.at(organism.species_id);

        std::vector<std::string> prey_ids, mate_ids, predator_ids;

        auto check_tile_occupants = [&](const world::WorldTile& observed_tile) {
            for (const auto& occupant_id : observed_tile.occupants) {
                if (occupant_id == organism.organism_id) continue;
                auto it = organisms.find(occupant_id);
                if (it == organisms.end() || !it->second.alive) continue;
                const auto& other = it->second;
                const auto& other_profile = species.at(other.species_id);
                if (foodweb_.can_predate(species_profile, other_profile)) {
                    prey_ids.push_back(other.organism_id);
                }
                if (foodweb_.can_predate(other_profile, species_profile)) {
                    predator_ids.push_back(other.organism_id);
                }
                if (other.species_id == organism.species_id &&
                    other.sex != organism.sex &&
                    other.ready_to_reproduce(other_profile)) {
                    mate_ids.push_back(other.organism_id);
                }
            }
        };

        check_tile_occupants(tile);
        for (auto* neighbor : neighbor_ptrs) {
            check_tile_occupants(*neighbor);
        }

        double local_density = competition_.population_pressure(tile);

        return behavior::PerceptionSnapshot{
            .tile_plants = tile.resources.plants,
            .tile_water = tile.resources.water,
            .nearby_prey_ids = std::move(prey_ids),
            .nearby_mate_ids = std::move(mate_ids),
            .nearby_predator_ids = std::move(predator_ids),
            .local_density = local_density,
            .season = clock.season(),
            .is_daylight = clock.is_daylight(),
        };
    }

    void execute_action(
        life::Organism& organism,
        const life::SpeciesProfile& profile,
        world::WorldTile& tile,
        const behavior::ActionDecision& decision,
        const behavior::PerceptionSnapshot& perception,
        std::map<std::tuple<int, int, std::string>, std::map<std::string, std::vector<std::string>>>& mating_intents
    ) {
        organism.behavior_state = behavior::action_to_string(decision.action);
        organism.last_action = decision.reason;

        using behavior::ActionType;
        switch (decision.action) {
            case ActionType::Drink: {
                double consumed = tile.resources.consume_water(profile.water_need * 20.0);
                organism.thirst = std::max(0.0, organism.thirst - consumed / 10.0);
                organism.energy = std::min(1.0, organism.energy + 0.05);
                break;
            }
            case ActionType::Graze: {
                double consumed = tile.resources.consume_plants(profile.grazing_rate);
                organism.hunger = std::max(0.0, organism.hunger - consumed / std::max(1.0, profile.grazing_rate));
                organism.energy = std::min(1.0, organism.energy + 0.08);
                break;
            }
            case ActionType::Hunt: {
                predation_.resolve(*this, organism, profile, perception);
                break;
            }
            case ActionType::Rest: {
                organism.energy = std::min(1.0, organism.energy + 0.18);
                organism.hunger = std::min(2.0, organism.hunger + 0.03);
                break;
            }
            case ActionType::SeekMate: {
                auto key = std::make_tuple(organism.position.first, organism.position.second, organism.species_id);
                mating_intents[key][organism.sex].push_back(organism.organism_id);
                organism.energy = std::max(0.0, organism.energy - 0.04);
                break;
            }
            case ActionType::Wander:
            case ActionType::Flee: {
                auto destination = migration_.choose_destination(world, organism, profile);
                world.move_organism(organism.organism_id, organism.position, destination);
                organism.position = destination;
                organism.energy = std::max(0.0, organism.energy - profile.movement_cost);
                organism.hunger = std::min(2.0, organism.hunger + 0.04);
                organism.thirst = std::min(2.0, organism.thirst + 0.03);
                break;
            }
            case ActionType::Idle:
            default: {
                organism.energy = std::min(1.0, organism.energy + 0.02);
                break;
            }
        }

        if (organism.energy <= 0.0) {
            organism.health = std::max(0.0, organism.health - 0.05);
        }
    }
};

} // namespace seeds::core
