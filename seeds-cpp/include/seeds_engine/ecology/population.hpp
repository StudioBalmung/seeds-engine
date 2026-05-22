#pragma once
#include <algorithm>
#include <map>
#include <string>

namespace seeds::ecology {

struct PopulationStats {
    int tick = 0;
    int total_alive = 0;
    int births = 0;
    int deaths = 0;
    std::map<std::string, int> species_counts;
    double average_health = 0.0;
    double average_energy = 0.0;
    double average_hunger = 0.0;
    double average_thirst = 0.0;
};

class PopulationTracker {
public:
    template <typename OrganismMap>
    PopulationStats summarize(const OrganismMap& organisms, int tick, int births, int deaths) {
        PopulationStats stats;
        stats.tick = tick;
        stats.births = births;
        stats.deaths = deaths;

        double sum_health = 0.0, sum_energy = 0.0, sum_hunger = 0.0, sum_thirst = 0.0;
        for (const auto& [id, organism] : organisms) {
            if (!organism.alive) continue;
            stats.total_alive++;
            stats.species_counts[organism.species_id]++;
            sum_health += organism.health;
            sum_energy += organism.energy;
            sum_hunger += organism.hunger;
            sum_thirst += organism.thirst;
        }

        int count = std::max(1, stats.total_alive);
        stats.average_health = sum_health / count;
        stats.average_energy = sum_energy / count;
        stats.average_hunger = sum_hunger / count;
        stats.average_thirst = sum_thirst / count;
        return stats;
    }
};

} // namespace seeds::ecology
