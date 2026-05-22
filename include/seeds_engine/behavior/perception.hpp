#pragma once
#include <string>
#include <vector>

namespace seeds::behavior {

struct PerceptionSnapshot {
    double tile_plants = 0.0;
    double tile_water = 0.0;
    std::vector<std::string> nearby_prey_ids;
    std::vector<std::string> nearby_mate_ids;
    std::vector<std::string> nearby_predator_ids;
    double local_density = 0.0;
    std::string season = "spring";
    bool is_daylight = true;
};

} // namespace seeds::behavior
