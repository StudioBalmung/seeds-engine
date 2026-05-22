#pragma once
#include <algorithm>
#include <array>
#include <string>

#include "../export.hpp"

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

class SEEDS_API SimulationClock {
public:
    explicit SimulationClock(TimeConfig config = {});

    void step(int ticks = 1);

    [[nodiscard]] double current_time_hours() const noexcept;
    [[nodiscard]] int hour_of_day() const noexcept;
    [[nodiscard]] int day_index() const noexcept;
    [[nodiscard]] std::string season() const;
    [[nodiscard]] bool is_daylight() const noexcept;
    [[nodiscard]] int tick() const noexcept { return tick_; }
    [[nodiscard]] const TimeConfig& config() const noexcept { return config_; }

private:
    TimeConfig config_;
    int tick_ = 0;
    int time_minutes_ = 0;
};

} // namespace seeds::core
