#pragma once
#include <algorithm>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace seeds::life {

/**
 * BeliefSystem defines a religion/spiritual practice in the simulation.
 * Each has tenets that influence organism behavior and social bonding.
 */
struct BeliefSystem {
    std::string belief_id;
    std::string name;
    double peace_bonus = 0.0;        // Reduces aggression (0.0 - 1.0)
    double cooperation_bonus = 0.0;  // Increases resource sharing likelihood
    double fertility_modifier = 0.0; // +/- effect on reproduction readiness
    double mortality_comfort = 0.0;  // Reduces stress from low health/elder age
    std::vector<std::string> tenets; // Descriptive list of beliefs
};

/**
 * FaithState tracks an individual organism's religious/spiritual state.
 */
struct FaithState {
    std::string belief_id;
    double devotion = 0.0;           // 0.0 (secular) to 1.0 (zealous)
    double community_bond = 0.0;     // Social attachment to co-believers
    int conversion_cooldown = 0;     // Ticks before next possible conversion
    int rituals_performed = 0;       // Lifetime ritual count
};

/**
 * FaithEngine handles belief adoption, ritual effects, faith spreading,
 * and social cohesion bonuses from shared religious practice.
 */
class FaithEngine {
public:
    void register_belief(BeliefSystem belief) {
        beliefs_[belief.belief_id] = std::move(belief);
    }

    void assign_belief(FaithState& faith, const std::string& belief_id, double initial_devotion) {
        if (!beliefs_.count(belief_id)) return;
        faith.belief_id = belief_id;
        faith.devotion = std::clamp(initial_devotion, 0.0, 1.0);
        faith.conversion_cooldown = 12;
    }

    /**
     * Update faith state each tick. Devotion drifts based on organism wellbeing.
     * High health/energy → devotion grows. High stress → devotion may wane or spike.
     */
    void update_tick(FaithState& faith, double health, double energy, double hunger,
                     double local_cobelievers_ratio, std::mt19937& rng) {
        if (faith.belief_id.empty()) return;

        // Community reinforcement
        faith.community_bond = std::clamp(
            faith.community_bond + (local_cobelievers_ratio - 0.3) * 0.05,
            0.0, 1.0
        );

        // Devotion drift
        double wellbeing = (health + energy) * 0.5 - hunger * 0.3;
        if (wellbeing > 0.6) {
            faith.devotion = std::clamp(faith.devotion + 0.01 + faith.community_bond * 0.02, 0.0, 1.0);
        } else if (wellbeing < 0.3) {
            // Crisis of faith OR increased devotion (random)
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            if (dist(rng) < 0.3 + faith.community_bond * 0.3) {
                faith.devotion = std::clamp(faith.devotion + 0.03, 0.0, 1.0);
            } else {
                faith.devotion = std::clamp(faith.devotion - 0.02, 0.0, 1.0);
            }
        }

        if (faith.conversion_cooldown > 0) {
            faith.conversion_cooldown--;
        }
    }

    /**
     * Perform a ritual if devotion is high enough. Grants temporary buffs.
     * Returns energy bonus (or 0 if no ritual performed).
     */
    double perform_ritual(FaithState& faith, double energy) {
        if (faith.belief_id.empty() || faith.devotion < 0.4 || energy < 0.3) return 0.0;
        faith.rituals_performed++;
        faith.devotion = std::clamp(faith.devotion + 0.05, 0.0, 1.0);
        faith.community_bond = std::clamp(faith.community_bond + 0.03, 0.0, 1.0);
        return 0.06; // energy bonus from spiritual fulfillment
    }

    /**
     * Attempt to spread belief to a target. Success depends on devotion,
     * community bond, and target's existing faith strength.
     */
    bool attempt_conversion(const FaithState& source, FaithState& target, std::mt19937& rng) {
        if (source.belief_id.empty()) return false;
        if (target.conversion_cooldown > 0) return false;
        if (target.belief_id == source.belief_id) return false;

        double influence = source.devotion * 0.4 + source.community_bond * 0.3;
        double resistance = target.devotion * 0.6 + target.community_bond * 0.2;
        double chance = std::clamp(influence - resistance + 0.1, 0.0, 0.6);

        std::uniform_real_distribution<double> dist(0.0, 1.0);
        if (dist(rng) < chance) {
            target.belief_id = source.belief_id;
            target.devotion = 0.2;
            target.community_bond = 0.1;
            target.conversion_cooldown = 24;
            return true;
        }
        target.conversion_cooldown = 6;
        return false;
    }

    /**
     * Get behavior modifiers from the organism's current belief system.
     */
    struct FaithModifiers {
        double aggression_reduction = 0.0;
        double cooperation_boost = 0.0;
        double fertility_modifier = 0.0;
        double stress_reduction = 0.0;
    };

    FaithModifiers get_modifiers(const FaithState& faith) const {
        FaithModifiers mods;
        if (faith.belief_id.empty()) return mods;
        auto it = beliefs_.find(faith.belief_id);
        if (it == beliefs_.end()) return mods;
        const auto& belief = it->second;
        double scale = faith.devotion;
        mods.aggression_reduction = belief.peace_bonus * scale;
        mods.cooperation_boost = belief.cooperation_bonus * scale;
        mods.fertility_modifier = belief.fertility_modifier * scale;
        mods.stress_reduction = belief.mortality_comfort * scale;
        return mods;
    }

    [[nodiscard]] const std::map<std::string, BeliefSystem>& beliefs() const noexcept {
        return beliefs_;
    }

    /**
     * Create some default belief systems for Phase 1 simulation.
     */
    void register_defaults() {
        register_belief(BeliefSystem{
            .belief_id = "sun_worship",
            .name = "Solar Devotion",
            .peace_bonus = 0.2,
            .cooperation_bonus = 0.3,
            .fertility_modifier = 0.1,
            .mortality_comfort = 0.15,
            .tenets = {"daylight is sacred", "share warmth", "celebrate harvests"},
        });
        register_belief(BeliefSystem{
            .belief_id = "ancestor_spirits",
            .name = "Ancestor Reverence",
            .peace_bonus = 0.35,
            .cooperation_bonus = 0.4,
            .fertility_modifier = 0.05,
            .mortality_comfort = 0.4,
            .tenets = {"honor the dead", "protect the lineage", "elders are wise"},
        });
        register_belief(BeliefSystem{
            .belief_id = "nature_harmony",
            .name = "Nature Harmony",
            .peace_bonus = 0.5,
            .cooperation_bonus = 0.25,
            .fertility_modifier = -0.05,
            .mortality_comfort = 0.25,
            .tenets = {"balance above all", "do not over-consume", "migrate with seasons"},
        });
        register_belief(BeliefSystem{
            .belief_id = "predator_cult",
            .name = "Predator Cult",
            .peace_bonus = -0.2,
            .cooperation_bonus = 0.1,
            .fertility_modifier = 0.15,
            .mortality_comfort = 0.05,
            .tenets = {"strength is virtue", "the hunt is prayer", "weakness is sin"},
        });
    }

private:
    std::map<std::string, BeliefSystem> beliefs_;
};

} // namespace seeds::life
