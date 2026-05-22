#pragma once
#include <algorithm>
#include <queue>
#include <vector>

#include "events.hpp"

namespace seeds::core {

class EventScheduler {
public:
    void schedule(SimulationEvent event) {
        queue_.push_back(std::move(event));
        std::push_heap(queue_.begin(), queue_.end(), comparator_);
    }

    std::vector<SimulationEvent> pop_due(int current_tick) {
        std::vector<SimulationEvent> due;
        while (!queue_.empty() && queue_.front().tick <= current_tick) {
            std::pop_heap(queue_.begin(), queue_.end(), comparator_);
            due.push_back(std::move(queue_.back()));
            queue_.pop_back();
        }
        return due;
    }

    [[nodiscard]] std::size_t size() const noexcept { return queue_.size(); }

private:
    static constexpr auto comparator_ = [](const SimulationEvent& a, const SimulationEvent& b) {
        return a > b;
    };
    std::vector<SimulationEvent> queue_;
};

} // namespace seeds::core
