#pragma once
#include <algorithm>
#include <map>
#include <random>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "../core/events.hpp"

namespace seeds::life {

struct BirthRequest {
    std::string mother_id;
    std::string father_id;
    std::string species_id;
    std::pair<int, int> position;
};

class ReproductionEngine {
public:
    explicit ReproductionEngine(std::mt19937& rng) : rng_(rng) {}

    template <typename Engine>
    void queue_births(Engine& engine,
                      std::map<std::tuple<int, int, std::string>, std::map<std::string, std::vector<std::string>>>& mating_intents) {
        for (auto& [key, grouping] : mating_intents) {
            auto& females_ids = grouping["female"];
            auto& males_ids = grouping["male"];

            std::vector<std::string> valid_females;
            std::vector<std::string> valid_males;
            for (auto& oid : females_ids) {
                if (engine.organisms.count(oid)) valid_females.push_back(oid);
            }
            for (auto& oid : males_ids) {
                if (engine.organisms.count(oid)) valid_males.push_back(oid);
            }

            if (valid_females.empty() || valid_males.empty()) continue;

            auto& female = engine.organisms[valid_females[0]];
            auto& male = engine.organisms[valid_males[0]];
            const auto& [x, y, species_id] = key;
            const auto& species = engine.species.at(species_id);

            std::uniform_int_distribution<int> litter_dist(species.litter_size_range.first, species.litter_size_range.second);
            int litter_size = litter_dist(rng_);

            for (int i = 0; i < litter_size; ++i) {
                core::SimulationEvent event;
                event.tick = engine.clock.tick() + 1;
                event.priority = 20;
                event.event_type = core::EventType::OrganismBirth;
                event.payload["mother_id"] = female.organism_id;
                event.payload["father_id"] = male.organism_id;
                event.payload["species_id"] = female.species_id;
                event.payload["position_x"] = std::to_string(female.position.first);
                event.payload["position_y"] = std::to_string(female.position.second);
                engine.scheduler.schedule(std::move(event));
            }

            female.reproduction_cooldown = species.gestation_ticks;
            male.reproduction_cooldown = std::max(1, species.gestation_ticks / 2);
            female.energy = std::max(0.0, female.energy - 0.25);
            male.energy = std::max(0.0, male.energy - 0.12);
            female.fertility = 0.0;
            male.fertility = std::max(0.0, male.fertility - 0.2);
        }
    }

private:
    std::mt19937& rng_;
};

} // namespace seeds::life
