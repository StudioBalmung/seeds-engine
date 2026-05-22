#pragma once
#include "../life/species.hpp"

namespace seeds::ecology {

class FoodWeb {
public:
    [[nodiscard]] bool can_predate(const life::SpeciesProfile& predator_species,
                                   const life::SpeciesProfile& prey_species) const {
        return predator_species.can_predate() && predator_species.species_id != prey_species.species_id;
    }
};

} // namespace seeds::ecology
