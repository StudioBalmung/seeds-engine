#include <iostream>
#include <sstream>
#include <string>

#include <seeds_engine/seeds_engine.hpp>
#include <seeds_engine/debug/debugger.hpp>

using namespace seeds;

static void print_help() {
    std::cout << R"(
Seeds Engine Debugger v1.0.0
============================
Commands:
  step [N]           - Advance N ticks (default: 1)
  status             - Show current tick, season, alive count
  snapshot           - Print world snapshot summary
  organism <id>      - Inspect an organism
  watch <id>         - Add organism to watch list
  unwatch <id>       - Remove organism from watch list
  watchlist          - Show watched organisms
  bp tick <N>        - Break at tick N
  bp pop <N>         - Break when population < N
  bp extinction      - Break on population extinction
  bp list            - List all breakpoints
  bp rm <id>         - Remove breakpoint
  spawn <species> <x> <y> [sex] [age]
                     - Spawn an organism
  disaster <tick> <x> <y> <radius> <severity>
                     - Schedule a disaster
  events [N]         - Show last N debug events (default: 20)
  flush              - Export all events as JSON
  belief <org_id> <belief_id> [devotion]
                     - Assign belief to organism
  help               - Show this help
  quit               - Exit debugger
)";
}

int main(int argc, char* argv[]) {
    int seed = 42;
    int width = 8, height = 8;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--seed" && i + 1 < argc) seed = std::stoi(argv[++i]);
        else if (arg == "--width" && i + 1 < argc) width = std::stoi(argv[++i]);
        else if (arg == "--height" && i + 1 < argc) height = std::stoi(argv[++i]);
    }

    auto facade = api::create_phase1_engine(seed, core::TimeConfig{},
                                            api::WorldConfig{.width = width, .height = height});
    debug::SeedsDebugger debugger(50000);
    debugger.enable();

    // Setup faith engine on the main engine
    life::FaithEngine faith_engine;
    faith_engine.register_defaults();

    // Track faith states per organism
    std::map<std::string, life::FaithState> faith_states;

    std::cout << "Seeds Engine Debugger v1.0.0 (seed=" << seed
              << ", world=" << width << "x" << height << ")\n";
    std::cout << "Type 'help' for commands.\n\n";

    std::string line;
    std::cout << "(seeds-dbg) " << std::flush;
    while (std::getline(std::cin, line)) {
        if (line.empty()) { std::cout << "(seeds-dbg) " << std::flush; continue; }
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "quit" || cmd == "exit" || cmd == "q") break;

        if (cmd == "help") {
            print_help();
        } else if (cmd == "step") {
            int n = 1;
            iss >> n;
            if (n < 1) n = 1;

            for (int i = 0; i < n; ++i) {
                auto stats = facade.step(1);
                auto& s = stats[0];

                // Log events
                debugger.log_info(facade.engine().clock.tick(), "step",
                    "alive=" + std::to_string(s.total_alive) +
                    " births=" + std::to_string(s.births) +
                    " deaths=" + std::to_string(s.deaths));

                // Check watched organisms
                for (const auto& oid : debugger.watch_list()) {
                    auto it = facade.engine().organisms.find(oid);
                    if (it != facade.engine().organisms.end()) {
                        auto& o = it->second;
                        debugger.log_info(facade.engine().clock.tick(), "watch",
                            oid + " h=" + std::to_string(o.health).substr(0, 5) +
                            " e=" + std::to_string(o.energy).substr(0, 5) +
                            " " + o.behavior_state);
                    }
                }

                // Check breakpoints
                std::vector<std::string> deaths;
                auto bp = debugger.check_breakpoints(
                    facade.engine().clock.tick(), s.total_alive, deaths);
                if (bp) {
                    std::cout << "*** BREAKPOINT HIT: " << bp->condition_type
                              << " = " << bp->value << " (id:" << bp->id << ")\n";
                    break;
                }
            }
            std::cout << "tick=" << facade.engine().clock.tick()
                      << " season=" << facade.engine().clock.season()
                      << " alive=" << facade.engine().population_history.back().total_alive << "\n";

        } else if (cmd == "status") {
            int alive = 0;
            for (const auto& [id, org] : facade.engine().organisms)
                if (org.alive) alive++;
            std::cout << "tick: " << facade.engine().clock.tick()
                      << " | season: " << facade.engine().clock.season()
                      << " | alive: " << alive << "\n";

        } else if (cmd == "snapshot") {
            auto snap = facade.snapshot();
            std::cout << "tick=" << std::get<int>(snap["tick"])
                      << " alive=" << std::get<int>(snap["total_alive"]) << "\n";
            auto& res = std::get<std::map<std::string, double>>(snap["world_resources"]);
            for (const auto& [k, v] : res) std::cout << "  " << k << "=" << v << "\n";

        } else if (cmd == "organism") {
            std::string oid;
            iss >> oid;
            auto it = facade.engine().organisms.find(oid);
            if (it == facade.engine().organisms.end()) {
                std::cout << "  Not found: " << oid << "\n";
            } else {
                auto& o = it->second;
                std::cout << "  id=" << o.organism_id << " species=" << o.species_id
                          << " sex=" << o.sex << " gen=" << o.generation << "\n"
                          << "  pos=(" << o.position.first << "," << o.position.second << ")"
                          << " alive=" << o.alive << " age=" << o.age_ticks << "\n"
                          << "  health=" << o.health << " energy=" << o.energy
                          << " hunger=" << o.hunger << " thirst=" << o.thirst << "\n"
                          << "  state=" << o.behavior_state << " action=" << o.last_action << "\n";
                auto fit = faith_states.find(oid);
                if (fit != faith_states.end() && !fit->second.belief_id.empty()) {
                    std::cout << "  belief=" << fit->second.belief_id
                              << " devotion=" << fit->second.devotion << "\n";
                }
            }

        } else if (cmd == "watch") {
            std::string oid; iss >> oid;
            debugger.watch(oid);
            std::cout << "  Watching: " << oid << "\n";

        } else if (cmd == "unwatch") {
            std::string oid; iss >> oid;
            debugger.unwatch(oid);
            std::cout << "  Unwatched: " << oid << "\n";

        } else if (cmd == "watchlist") {
            for (const auto& oid : debugger.watch_list())
                std::cout << "  " << oid << "\n";
            if (debugger.watch_list().empty()) std::cout << "  (empty)\n";

        } else if (cmd == "bp") {
            std::string sub; iss >> sub;
            if (sub == "tick") {
                int t; iss >> t;
                int id = debugger.add_breakpoint("tick", std::to_string(t));
                std::cout << "  Breakpoint " << id << ": tick=" << t << "\n";
            } else if (sub == "pop") {
                int p; iss >> p;
                int id = debugger.add_breakpoint("population_below", std::to_string(p));
                std::cout << "  Breakpoint " << id << ": population<" << p << "\n";
            } else if (sub == "extinction") {
                int id = debugger.add_breakpoint("extinction", "0");
                std::cout << "  Breakpoint " << id << ": extinction\n";
            } else if (sub == "list") {
                for (const auto& bp : debugger.breakpoints())
                    std::cout << "  [" << bp.id << "] " << bp.condition_type
                              << "=" << bp.value << (bp.enabled ? "" : " (disabled)") << "\n";
            } else if (sub == "rm") {
                int id; iss >> id;
                debugger.remove_breakpoint(id);
                std::cout << "  Removed breakpoint " << id << "\n";
            }

        } else if (cmd == "spawn") {
            std::string species; int x, y; std::string sex = ""; int age = 24;
            iss >> species >> x >> y;
            if (iss >> sex) { iss >> age; }
            facade.spawn(api::SpawnRequest{.species_id = species, .x = x, .y = y,
                                           .sex = sex.empty() ? std::nullopt : std::optional(sex),
                                           .age_ticks = age});
            std::cout << "  Spawned " << species << " at (" << x << "," << y << ")\n";

        } else if (cmd == "disaster") {
            int tick, x, y, radius; double severity;
            iss >> tick >> x >> y >> radius >> severity;
            facade.engine().schedule_disaster(tick, {x, y}, radius, severity);
            std::cout << "  Scheduled disaster at tick=" << tick << "\n";

        } else if (cmd == "events") {
            int n = 20; iss >> n;
            auto& evts = debugger.events();
            int start = std::max(0, (int)evts.size() - n);
            for (int i = start; i < (int)evts.size(); ++i) {
                auto& e = evts[i];
                std::cout << "  [" << e.tick << "] " << e.severity << " "
                          << e.category << ": " << e.message << "\n";
            }

        } else if (cmd == "flush") {
            std::cout << debugger.to_json() << "\n";

        } else if (cmd == "belief") {
            std::string oid, belief_id; double devotion = 0.5;
            iss >> oid >> belief_id;
            iss >> devotion;
            auto& fs = faith_states[oid];
            faith_engine.assign_belief(fs, belief_id, devotion);
            std::cout << "  " << oid << " now believes in " << belief_id
                      << " (devotion=" << devotion << ")\n";

        } else {
            std::cout << "  Unknown command. Type 'help'.\n";
        }

        std::cout << "(seeds-dbg) " << std::flush;
    }

    std::cout << "\nDebugger exiting. " << debugger.event_count() << " events recorded.\n";
    return 0;
}
