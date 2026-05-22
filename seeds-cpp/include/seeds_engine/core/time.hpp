#pragma once
#include <algorithm>
#include <array>
#include <string>

namespace seeds::core {

inline constexpr std::array<const char*, 4> SEASON_CYCLE = {"spring", "summer", "autumn", "winter"};

struct TimeConfig {
    int tick_minutes = 60;
    int wall_seconds_per_tick = 600;
    int season_length_days = 30;
    int daylight_start_hour = 6;
    int daylight_end_hour = 18;

    [[nodiscard]] constexpr int seconds_per_tick() const noexcept {
        return tick_minutes * 60;
    }

    [[nodiscard]] constexpr int ticks_per_day() const noexcept {
        int minutes_per_day = 24 * 60;
        return std::max(1, minutes_per_day / tick_minutes);
    }
};

class SimulationClock {
public:
    explicit SimulationClock(TimeConfig config = {})
        : config_(config) {}

    void step(int ticks = 1) {
        tick_ += ticks;
        time_minutes_ += ticks * config_.tick_minutes;
    }

    [[nodiscard]] double current_time_hours() const noexcept {
        return time_minutes_ / 60.0;
    }

    [[nodiscard]] int hour_of_day() const noexcept {
        return static_cast<int>(current_time_hours()) % 24;
    }

    [[nodiscard]] int day_index() const noexcept {
        return time_minutes_ / (24 * 60);
    }

    [[nodiscard]] std::string season() const {
        int season_index = (day_index() / config_.season_length_days) % 4;
        return SEASON_CYCLE[season_index];
    }

    [[nodiscard]] bool is_daylight() const noexcept {
        return config_.daylight_start_hour <= hour_of_day() && hour_of_day() < config_.daylight_end_hour;
    }

    [[nodiscard]] int tick() const noexcept { return tick_; }
    [[nodiscard]] const TimeConfig& config() const noexcept { return config_; }

private:
    TimeConfig config_;
    int tick_ = 0;
    int time_minutes_ = 0;
};

} // namespace seeds::core
