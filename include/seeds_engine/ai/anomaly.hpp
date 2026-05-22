#pragma once
#include <map>
#include <string>
#include <vector>

namespace seeds::ai {

struct AnomalyFinding {
    std::string severity;
    std::string reason;
};

class AnomalyDetector {
public:
    template <typename Engine>
    std::vector<AnomalyFinding> detect(const Engine& engine) const {
        if (engine.population_history.empty()) return {};
        const auto& latest = engine.population_history.back();
        std::vector<AnomalyFinding> findings;
        if (latest.total_alive == 0) {
            findings.push_back({"high", "population extinction"});
        }
        if (latest.average_hunger > 0.9) {
            findings.push_back({"medium", "widespread starvation pressure"});
        }
        if (latest.average_thirst > 0.9) {
            findings.push_back({"medium", "widespread dehydration pressure"});
        }
        return findings;
    }
};

} // namespace seeds::ai
