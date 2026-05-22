#pragma once
#include <algorithm>
#include <random>
#include <string>

namespace seeds::life {

struct DiseaseState {
    bool infected = false;
    std::string pathogen_name;
    double severity = 0.0;
    double immunity = 0.0;
};

class DiseaseEngine {
public:
    template <typename Organism>
    void update(Organism& organism, int tile_density, std::mt19937& rng) {
        auto& disease = organism.disease;
        double stress = std::max(0.0, organism.hunger - 0.75) + std::max(0.0, organism.thirst - 0.75);

        if (disease.infected) {
            disease.severity = clamp(disease.severity + 0.03 + stress * 0.05, 0.0, 1.0);
            organism.health = clamp(organism.health - disease.severity * 0.04, 0.0, 1.0);
            disease.immunity = clamp(disease.immunity + 0.02, 0.0, 1.0);
            if (disease.severity < 0.2 && disease.immunity > 0.75) {
                disease.infected = false;
                disease.pathogen_name.clear();
                disease.severity = 0.0;
            }
            return;
        }

        double infection_pressure = std::max(0, tile_density - 3) * 0.015 + stress * 0.03;
        double infection_chance = infection_pressure * (1.0 - disease.immunity);
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        if (infection_chance > dist(rng)) {
            disease.infected = true;
            disease.pathogen_name = "opportunistic_pathogen";
            disease.severity = 0.15;
        } else {
            disease.immunity = clamp(disease.immunity + 0.003, 0.0, 1.0);
        }
    }

private:
    static double clamp(double value, double low, double high) {
        return std::max(low, std::min(high, value));
    }
};

} // namespace seeds::life
