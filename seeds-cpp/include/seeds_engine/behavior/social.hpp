#pragma once
#include <algorithm>

namespace seeds::behavior {

inline double local_density_score(int occupants, int carrying_capacity) {
    if (carrying_capacity <= 0) return 0.0;
    return static_cast<double>(occupants) / carrying_capacity;
}

} // namespace seeds::behavior
