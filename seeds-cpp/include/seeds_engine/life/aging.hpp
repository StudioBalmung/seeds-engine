#pragma once
#include <algorithm>
#include <string>

#include "species.hpp"

namespace seeds::life {

enum class AgeStage {
    Infant,
    Juvenile,
    Adult,
    Elder,
};

inline std::string age_stage_to_string(AgeStage s) {
    switch (s) {
        case AgeStage::Infant:   return "infant";
        case AgeStage::Juvenile: return "juvenile";
        case AgeStage::Adult:    return "adult";
        case AgeStage::Elder:    return "elder";
    }
    return "unknown";
}

class AgingModel {
public:
    [[nodiscard]] AgeStage resolve_stage(int age_ticks, const SpeciesProfile& species) const noexcept {
        if (age_ticks <= std::max(1, species.mature_age_ticks / 5))
            return AgeStage::Infant;
        if (age_ticks < species.mature_age_ticks)
            return AgeStage::Juvenile;
        if (age_ticks < static_cast<int>(species.max_age_ticks * 0.8))
            return AgeStage::Adult;
        return AgeStage::Elder;
    }
};

} // namespace seeds::life
