#pragma once
#include <string>

namespace seeds::behavior {

enum class ActionType {
    Graze,
    Hunt,
    Drink,
    Rest,
    Wander,
    SeekMate,
    Flee,
    Idle,
};

inline std::string action_to_string(ActionType a) {
    switch (a) {
        case ActionType::Graze:    return "graze";
        case ActionType::Hunt:     return "hunt";
        case ActionType::Drink:    return "drink";
        case ActionType::Rest:     return "rest";
        case ActionType::Wander:   return "wander";
        case ActionType::SeekMate: return "seek_mate";
        case ActionType::Flee:     return "flee";
        case ActionType::Idle:     return "idle";
    }
    return "idle";
}

struct ActionDecision {
    ActionType action = ActionType::Idle;
    std::string reason;
};

} // namespace seeds::behavior
