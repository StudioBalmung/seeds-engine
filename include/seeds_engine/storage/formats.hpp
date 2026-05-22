#pragma once
#include <sstream>
#include <string>

#include "snapshots.hpp"

namespace seeds::storage {

// Minimal JSON-like serialization (no external dependency)
inline std::string to_json_simple(const Snapshot& snapshot) {
    std::ostringstream oss;
    oss << "{\n";
    bool first = true;
    for (const auto& [key, value] : snapshot) {
        if (!first) oss << ",\n";
        first = false;
        oss << "  \"" << key << "\": ";
        std::visit([&oss](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, int>) {
                oss << v;
            } else if constexpr (std::is_same_v<T, double>) {
                oss << v;
            } else if constexpr (std::is_same_v<T, std::string>) {
                oss << "\"" << v << "\"";
            } else if constexpr (std::is_same_v<T, std::map<std::string, int>>) {
                oss << "{";
                bool f = true;
                for (const auto& [k, val] : v) {
                    if (!f) oss << ", ";
                    f = false;
                    oss << "\"" << k << "\": " << val;
                }
                oss << "}";
            } else if constexpr (std::is_same_v<T, std::map<std::string, double>>) {
                oss << "{";
                bool f = true;
                for (const auto& [k, val] : v) {
                    if (!f) oss << ", ";
                    f = false;
                    oss << "\"" << k << "\": " << val;
                }
                oss << "}";
            } else if constexpr (std::is_same_v<T, std::vector<std::map<std::string, std::string>>>) {
                oss << "[...]";
            }
        }, value);
    }
    oss << "\n}";
    return oss.str();
}

} // namespace seeds::storage
