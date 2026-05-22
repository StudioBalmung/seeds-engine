#pragma once
#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "../export.hpp"

namespace seeds::net {

/**
 * MCP (Model Context Protocol) Server for Seeds Engine.
 *
 * Provides a JSON-RPC 2.0 compatible interface over stdin/stdout for
 * integration with AI assistants, LLM tool-calling, and automation pipelines.
 *
 * Protocol: Reads newline-delimited JSON from stdin, writes JSON responses to stdout.
 *
 * Supported methods:
 *   - initialize          → handshake and capabilities
 *   - tools/list          → list available simulation tools
 *   - tools/call          → invoke a simulation tool by name
 *   - notifications/initialized → client ready acknowledgment
 *
 * Tools exposed:
 *   - seeds.step           → advance simulation N ticks
 *   - seeds.spawn          → spawn an organism
 *   - seeds.snapshot       → get world state JSON
 *   - seeds.analytics      → get analytics report
 *   - seeds.disaster       → schedule a disaster event
 *   - seeds.get_tick       → query current tick
 *   - seeds.get_season     → query current season
 *   - seeds.alive_count    → count living organisms
 *   - seeds.organism_info  → get details of a specific organism
 *   - seeds.set_belief     → assign a belief to an organism
 */

struct McpToolDef {
    std::string name;
    std::string description;
    std::string input_schema_json;
};

using McpToolHandler = std::function<std::string(const std::map<std::string, std::string>& params)>;

class SEEDS_API McpServer {
public:
    McpServer() {
        register_builtin_tools();
    }

    void register_tool(McpToolDef tool_def, McpToolHandler handler) {
        tools_.push_back(std::move(tool_def));
        handlers_[tools_.back().name] = std::move(handler);
    }

    void set_handler(const std::string& tool_name, McpToolHandler handler) {
        handlers_[tool_name] = std::move(handler);
    }

    /**
     * Run the MCP server loop (blocking). Reads from stdin, writes to stdout.
     * Call this from a dedicated thread or as main() of a server process.
     */
    void run() {
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.empty()) continue;
            std::string response = handle_request(line);
            if (!response.empty()) {
                std::cout << response << "\n" << std::flush;
            }
        }
    }

    /**
     * Process a single JSON-RPC request string. Returns response JSON.
     * Useful for testing or embedding without stdin/stdout.
     */
    std::string handle_request(const std::string& request_json) {
        auto method = extract_json_string(request_json, "method");
        auto id = extract_json_string(request_json, "id");

        if (method == "initialize") {
            return make_response(id, make_initialize_result());
        }
        if (method == "notifications/initialized") {
            return ""; // no response for notifications
        }
        if (method == "tools/list") {
            return make_response(id, make_tools_list());
        }
        if (method == "tools/call") {
            auto tool_name = extract_nested_string(request_json, "params", "name");
            auto arguments = extract_arguments(request_json);
            return make_response(id, call_tool(tool_name, arguments));
        }
        return make_error_response(id, -32601, "Method not found: " + method);
    }

    [[nodiscard]] const std::vector<McpToolDef>& tools() const noexcept {
        return tools_;
    }

private:
    std::vector<McpToolDef> tools_;
    std::map<std::string, McpToolHandler> handlers_;

    void register_builtin_tools() {
        tools_.push_back({"seeds.step", "Advance simulation by N ticks",
            R"({"type":"object","properties":{"ticks":{"type":"integer","default":1}}})"});
        tools_.push_back({"seeds.spawn", "Spawn an organism",
            R"({"type":"object","properties":{"species_id":{"type":"string"},"x":{"type":"integer"},"y":{"type":"integer"},"sex":{"type":"string"},"age_ticks":{"type":"integer"}},"required":["species_id","x","y"]})"});
        tools_.push_back({"seeds.snapshot", "Get full world state as JSON",
            R"({"type":"object","properties":{}})"});
        tools_.push_back({"seeds.analytics", "Get analytics report",
            R"({"type":"object","properties":{}})"});
        tools_.push_back({"seeds.disaster", "Schedule a disaster",
            R"({"type":"object","properties":{"tick":{"type":"integer"},"x":{"type":"integer"},"y":{"type":"integer"},"radius":{"type":"integer"},"severity":{"type":"number"}},"required":["tick","x","y","radius","severity"]})"});
        tools_.push_back({"seeds.get_tick", "Get current simulation tick",
            R"({"type":"object","properties":{}})"});
        tools_.push_back({"seeds.get_season", "Get current season",
            R"({"type":"object","properties":{}})"});
        tools_.push_back({"seeds.alive_count", "Count living organisms",
            R"({"type":"object","properties":{}})"});
        tools_.push_back({"seeds.organism_info", "Get organism details",
            R"({"type":"object","properties":{"organism_id":{"type":"string"}},"required":["organism_id"]})"});
        tools_.push_back({"seeds.set_belief", "Assign belief to an organism",
            R"({"type":"object","properties":{"organism_id":{"type":"string"},"belief_id":{"type":"string"},"devotion":{"type":"number"}},"required":["organism_id","belief_id"]})"});
    }

    std::string call_tool(const std::string& name, const std::map<std::string, std::string>& params) {
        auto it = handlers_.find(name);
        if (it == handlers_.end()) {
            return R"({"content":[{"type":"text","text":"Error: tool not bound: )" + name + R"("}],"isError":true})";
        }
        try {
            std::string result = it->second(params);
            return R"({"content":[{"type":"text","text":)" + escape_json_value(result) + R"(}]})";
        } catch (const std::exception& e) {
            return R"({"content":[{"type":"text","text":"Exception: )" + std::string(e.what()) + R"("}],"isError":true})";
        }
    }

    std::string make_initialize_result() {
        return R"({"protocolVersion":"2024-11-05","capabilities":{"tools":{"listChanged":false}},"serverInfo":{"name":"seeds-engine","version":"1.0.0"}})";
    }

    std::string make_tools_list() {
        std::ostringstream oss;
        oss << R"({"tools":[)";
        for (size_t i = 0; i < tools_.size(); ++i) {
            if (i > 0) oss << ",";
            oss << R"({"name":")" << tools_[i].name
                << R"(","description":")" << tools_[i].description
                << R"(","inputSchema":)" << tools_[i].input_schema_json << "}";
        }
        oss << "]}";
        return oss.str();
    }

    static std::string make_response(const std::string& id, const std::string& result) {
        return R"({"jsonrpc":"2.0","id":)" + id + R"(,"result":)" + result + "}";
    }

    static std::string make_error_response(const std::string& id, int code, const std::string& message) {
        return R"({"jsonrpc":"2.0","id":)" + id + R"(,"error":{"code":)" +
               std::to_string(code) + R"(,"message":")" + message + R"("}})";
    }

    static std::string escape_json_value(const std::string& s) {
        std::ostringstream oss;
        oss << "\"";
        for (char c : s) {
            switch (c) {
                case '"':  oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:   oss << c; break;
            }
        }
        oss << "\"";
        return oss.str();
    }

    // Minimal JSON parsing helpers (no external dependency)
    static std::string extract_json_string(const std::string& json, const std::string& key) {
        std::string needle = "\"" + key + "\"";
        auto pos = json.find(needle);
        if (pos == std::string::npos) return "";
        pos = json.find(':', pos + needle.size());
        if (pos == std::string::npos) return "";
        pos++;
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        if (pos < json.size() && json[pos] == '"') {
            pos++;
            std::string result;
            while (pos < json.size() && json[pos] != '"') {
                if (json[pos] == '\\' && pos + 1 < json.size()) { result += json[++pos]; }
                else { result += json[pos]; }
                pos++;
            }
            return result;
        }
        // Number or other literal
        std::string result;
        while (pos < json.size() && json[pos] != ',' && json[pos] != '}' && json[pos] != ' ') {
            result += json[pos++];
        }
        return result;
    }

    static std::string extract_nested_string(const std::string& json,
                                             const std::string& outer_key,
                                             const std::string& inner_key) {
        auto outer_pos = json.find("\"" + outer_key + "\"");
        if (outer_pos == std::string::npos) return "";
        auto sub = json.substr(outer_pos);
        return extract_json_string(sub, inner_key);
    }

    static std::map<std::string, std::string> extract_arguments(const std::string& json) {
        std::map<std::string, std::string> result;
        auto args_pos = json.find("\"arguments\"");
        if (args_pos == std::string::npos) return result;
        auto brace = json.find('{', args_pos + 11);
        if (brace == std::string::npos) return result;
        int depth = 0;
        std::string sub;
        for (size_t i = brace; i < json.size(); ++i) {
            if (json[i] == '{') depth++;
            else if (json[i] == '}') { depth--; if (depth == 0) { sub = json.substr(brace, i - brace + 1); break; } }
        }
        // Parse flat key-value pairs from sub
        size_t pos = 0;
        while (pos < sub.size()) {
            auto kstart = sub.find('"', pos);
            if (kstart == std::string::npos) break;
            auto kend = sub.find('"', kstart + 1);
            if (kend == std::string::npos) break;
            std::string key = sub.substr(kstart + 1, kend - kstart - 1);
            auto colon = sub.find(':', kend + 1);
            if (colon == std::string::npos) break;
            pos = colon + 1;
            while (pos < sub.size() && sub[pos] == ' ') pos++;
            std::string value;
            if (pos < sub.size() && sub[pos] == '"') {
                pos++;
                while (pos < sub.size() && sub[pos] != '"') {
                    if (sub[pos] == '\\' && pos + 1 < sub.size()) { value += sub[++pos]; }
                    else { value += sub[pos]; }
                    pos++;
                }
                pos++;
            } else {
                while (pos < sub.size() && sub[pos] != ',' && sub[pos] != '}') {
                    value += sub[pos++];
                }
            }
            result[key] = value;
            pos++;
        }
        return result;
    }
};

} // namespace seeds::net
