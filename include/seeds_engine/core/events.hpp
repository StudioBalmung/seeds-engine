#pragma once
#include <any>
#include <compare>
#include <map>
#include <string>

namespace seeds::core {

enum class EventType {
    OrganismBirth,
    OrganismDeath,
    WorldEvent,
    Disaster,
};

struct SimulationEvent {
    int tick = 0;
    int priority = 0;
    EventType event_type = EventType::WorldEvent;
    std::map<std::string, std::string> payload;

    auto operator<=>(const SimulationEvent& other) const noexcept {
        if (auto cmp = tick <=> other.tick; cmp != 0) return cmp;
        return priority <=> other.priority;
    }
    bool operator==(const SimulationEvent& other) const noexcept {
        return tick == other.tick && priority == other.priority;
    }
};

struct EventRecord {
    int tick = 0;
    EventType event_type = EventType::WorldEvent;
    std::map<std::string, std::string> details;
};

} // namespace seeds::core
