#pragma once
#include <optional>
#include <vector>

#include "snapshots.hpp"

namespace seeds::storage {

class InMemorySnapshotRepository {
public:
    void save(Snapshot snapshot) {
        snapshots_.push_back(std::move(snapshot));
    }

    [[nodiscard]] std::optional<Snapshot> latest() const {
        if (snapshots_.empty()) return std::nullopt;
        return snapshots_.back();
    }

    [[nodiscard]] const std::vector<Snapshot>& all() const noexcept {
        return snapshots_;
    }

private:
    std::vector<Snapshot> snapshots_;
};

} // namespace seeds::storage
