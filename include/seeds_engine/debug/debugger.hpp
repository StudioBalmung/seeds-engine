#pragma once
#include <chrono>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "../export.hpp"

namespace seeds::debug {

/**
 * DebugEvent captures a single noteworthy occurrence during simulation.
 */
struct DebugEvent {
    int tick = 0;
    std::string category;    // "birth", "death", "disease", "predation", "disaster", "faith", "movement"
    std::string severity;    // "info", "warn", "error"
    std::string message;
    std::map<std::string, std::string> context;
};

/**
 * Breakpoint defines a condition that pauses the debugger.
 */
struct Breakpoint {
    int id = 0;
    std::string condition_type; // "tick", "population_below", "extinction", "organism_death"
    std::string value;
    bool enabled = true;
};

/**
 * SeedsDebugger — standalone runtime debugging and inspection tool.
 *
 * Features:
 *   - Event logging with severity levels
 *   - Breakpoints (tick-based, condition-based)
 *   - Watch expressions on organisms
 *   - Tick stepping with pause/resume
 *   - Event buffer with configurable max size
 *   - JSON export of debug session
 */
class SEEDS_API SeedsDebugger {
public:
    explicit SeedsDebugger(size_t max_buffer_size = 10000)
        : max_buffer_size_(max_buffer_size) {}

    void enable() noexcept { enabled_ = true; }
    void disable() noexcept { enabled_ = false; }
    [[nodiscard]] bool is_enabled() const noexcept { return enabled_; }

    /** Log a debug event. Only records if enabled. */
    void log(DebugEvent event) {
        if (!enabled_) return;
        events_.push_back(std::move(event));
        while (events_.size() > max_buffer_size_) {
            events_.pop_front();
        }
    }

    void log_info(int tick, const std::string& category, const std::string& message,
                  std::map<std::string, std::string> context = {}) {
        log(DebugEvent{tick, category, "info", message, std::move(context)});
    }

    void log_warn(int tick, const std::string& category, const std::string& message,
                  std::map<std::string, std::string> context = {}) {
        log(DebugEvent{tick, category, "warn", message, std::move(context)});
    }

    void log_error(int tick, const std::string& category, const std::string& message,
                   std::map<std::string, std::string> context = {}) {
        log(DebugEvent{tick, category, "error", message, std::move(context)});
    }

    /** Add a breakpoint. Returns the breakpoint ID. */
    int add_breakpoint(const std::string& condition_type, const std::string& value) {
        int id = next_bp_id_++;
        breakpoints_.push_back(Breakpoint{id, condition_type, value, true});
        return id;
    }

    void remove_breakpoint(int id) {
        breakpoints_.erase(
            std::remove_if(breakpoints_.begin(), breakpoints_.end(),
                           [id](const Breakpoint& bp) { return bp.id == id; }),
            breakpoints_.end()
        );
    }

    void toggle_breakpoint(int id) {
        for (auto& bp : breakpoints_) {
            if (bp.id == id) { bp.enabled = !bp.enabled; break; }
        }
    }

    /** Check if any breakpoint triggers. Returns the triggered BP or nullptr. */
    const Breakpoint* check_breakpoints(int current_tick, int alive_count,
                                        const std::vector<std::string>& recent_deaths) const {
        for (const auto& bp : breakpoints_) {
            if (!bp.enabled) continue;
            if (bp.condition_type == "tick" && std::to_string(current_tick) == bp.value) return &bp;
            if (bp.condition_type == "population_below" && alive_count < std::stoi(bp.value)) return &bp;
            if (bp.condition_type == "extinction" && alive_count == 0) return &bp;
            if (bp.condition_type == "organism_death") {
                for (const auto& death : recent_deaths) {
                    if (death == bp.value) return &bp;
                }
            }
        }
        return nullptr;
    }

    /** Add an organism to the watch list. */
    void watch(const std::string& organism_id) {
        watch_list_.insert(organism_id);
    }

    void unwatch(const std::string& organism_id) {
        watch_list_.erase(organism_id);
    }

    [[nodiscard]] const std::set<std::string>& watch_list() const noexcept {
        return watch_list_;
    }

    /** Flush events: returns all buffered events and clears buffer. */
    std::vector<DebugEvent> flush() {
        std::vector<DebugEvent> result(events_.begin(), events_.end());
        events_.clear();
        return result;
    }

    /** Get events without clearing (for inspection). */
    [[nodiscard]] const std::deque<DebugEvent>& events() const noexcept {
        return events_;
    }

    /** Export debug log as JSON string. */
    std::string to_json() const {
        std::ostringstream oss;
        oss << "[";
        bool first = true;
        for (const auto& ev : events_) {
            if (!first) oss << ",";
            first = false;
            oss << "{\"tick\":" << ev.tick
                << ",\"category\":\"" << ev.category
                << "\",\"severity\":\"" << ev.severity
                << "\",\"message\":\"" << escape(ev.message) << "\"";
            if (!ev.context.empty()) {
                oss << ",\"context\":{";
                bool cf = true;
                for (const auto& [k, v] : ev.context) {
                    if (!cf) oss << ",";
                    cf = false;
                    oss << "\"" << k << "\":\"" << escape(v) << "\"";
                }
                oss << "}";
            }
            oss << "}";
        }
        oss << "]";
        return oss.str();
    }

    [[nodiscard]] size_t event_count() const noexcept { return events_.size(); }
    [[nodiscard]] const std::vector<Breakpoint>& breakpoints() const noexcept { return breakpoints_; }

private:
    bool enabled_ = false;
    size_t max_buffer_size_;
    std::deque<DebugEvent> events_;
    std::vector<Breakpoint> breakpoints_;
    std::set<std::string> watch_list_;
    int next_bp_id_ = 1;

    static std::string escape(const std::string& s) {
        std::string out;
        for (char c : s) {
            switch (c) {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                default: out += c; break;
            }
        }
        return out;
    }
};

} // namespace seeds::debug
