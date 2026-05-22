# Seeds Engine v1.0.0 (Simulation Ecology Entity Dynamic System)

Seeds Engine is a deterministic, seeded ecology simulation library written in C++20.
It ships as a **shared/dynamic library** (`.dll` / `.so` / `.dylib`) with an optional
**C ABI** (`extern "C"`) for FFI integration with other languages.

## What's new in v1.0.0

- **C ABI** — `seeds_engine_c.h` / `libseeds_engine_c.dll` for embedding from C#, Rust, Zig, Lua, Java, and JavaScript
- **Faith module** — NPC religion/belief simulation with devotion, rituals, conversion, and behavior modifiers
- **MCP Server** — JSON-RPC 2.0 Model Context Protocol server for AI assistant / LLM tool-calling integration
- **Debugger Tool** — standalone interactive CLI debugger with breakpoints, watch list, and event logging
- **Bug fixes** — deferred death processing to prevent iterator invalidation; safe organism lookup during step

## Backend layers

| Layer | Directory | Contents |
|-------|-----------|----------|
| Core | `core/` | Engine loop, clock, events, scheduler |
| World | `world/` | Grid map, chunking, biome, terrain, climate, resources |
| Life | `life/` | Species, organisms, genetics, physiology, disease, aging, reproduction, **faith** |
| Behavior | `behavior/` | Perception, utility AI, social density |
| Ecology | `ecology/` | Predation, migration, competition, population, foodweb |
| Storage | `storage/` | Snapshots, repository, JSON serialization |
| AI | `ai/` | Anomaly detection, clustering, carrying capacity prediction, reports |
| Debug | `debug/` | Event logger, breakpoints, watch list, JSON export |
| Net | `net/` | MCP (Model Context Protocol) server |
| API | `api/` | Facade, schemas, convenience factory |

## Requirements

- **C++20** compiler (GCC 11+, Clang 14+, MSVC 2022+)
- **CMake 3.20+**
- No external dependencies

## Build & Run

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/seeds_demo --ticks 24
./build/seeds_tests
./build/seeds_debugger
```

Windows (MinGW / MSYS2):
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
.\build\seeds_demo.exe --ticks 24
.\build\seeds_tests.exe
.\build\seeds_debugger.exe
```

### CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `SEEDS_BUILD_CAPI` | `ON` | Build `libseeds_engine_c` (C ABI shared library) |
| `SEEDS_BUILD_DEBUGGER` | `ON` | Build `seeds_debugger` (standalone CLI tool) |
| `SEEDS_BUILD_TESTS` | `ON` | Build `seeds_tests` |

Disable optional targets: `cmake -B build -DSEEDS_BUILD_CAPI=OFF -DSEEDS_BUILD_DEBUGGER=OFF`

## Build outputs

| File | Description |
|------|-------------|
| `libseeds_engine.dll` | C++ shared library |
| `libseeds_engine_c.dll` | C ABI shared library (for FFI) |
| `seeds_demo` | Demo simulation executable |
| `seeds_debugger` | Interactive debugger CLI |
| `seeds_tests` | Test suite |

## C++ embedding example

```cpp
#include <seeds_engine/seeds_engine.hpp>
using namespace seeds;

int main() {
    auto facade = api::create_phase1_engine(42);
    facade.spawn(api::SpawnRequest{.species_id = "herbivore", .x = 2, .y = 2, .sex = "female", .age_ticks = 24});
    facade.spawn(api::SpawnRequest{.species_id = "herbivore", .x = 2, .y = 2, .sex = "male",   .age_ticks = 24});
    facade.spawn(api::SpawnRequest{.species_id = "carnivore", .x = 5, .y = 5, .age_ticks = 24});
    facade.step(10);

    // Faith module
    auto& org = facade.engine().organisms.begin()->second;
    facade.engine().faith_engine().assign_belief(org.faith, "sun_worship", 0.6);

    // Debugger
    facade.engine().debugger().enable();
    facade.step(5);
    std::string log_json = facade.engine().debugger().to_json();
    return 0;
}
```

## C ABI (FFI) — for C#, Rust, Zig, Lua, Java, JavaScript

Link against `libseeds_engine_c.dll/.so` and include `seeds_engine_c.h`.

```c
#include <seeds_engine/seeds_engine_c.h>

int main(void) {
    SeedsEngineHandle engine = seeds_create(42, 8, 8, 60);
    seeds_spawn(engine, "herbivore", 2, 2, "female", 24);
    seeds_spawn(engine, "herbivore", 2, 2, "male",   24);
    seeds_step(engine, 10);

    int alive = seeds_get_alive_count(engine);
    const char* season = seeds_get_season(engine);

    // Faith
    seeds_set_belief(engine, "O000001", "sun_worship", 0.7);

    // JSON snapshot (caller must free)
    char* json = seeds_snapshot_json(engine);
    seeds_free_string(json);

    seeds_destroy(engine);
    return 0;
}
```

### C# (.NET 10+)
```csharp
using System.Runtime.InteropServices;

partial class SeedsEngine {
    [LibraryImport("seeds_engine_c", StringMarshalling = StringMarshalling.Utf8)]
    public static partial IntPtr seeds_create(int seed, int w, int h, int tickMin);

    [LibraryImport("seeds_engine_c", StringMarshalling = StringMarshalling.Utf8)]
    public static partial void seeds_step(IntPtr handle, int ticks);

    [LibraryImport("seeds_engine_c", StringMarshalling = StringMarshalling.Utf8)]
    public static partial int seeds_get_alive_count(IntPtr handle);

    [LibraryImport("seeds_engine_c")]
    public static partial void seeds_destroy(IntPtr handle);
}
```

### Rust (1.95+)
```rust
extern "C" {
    fn seeds_create(seed: i32, w: i32, h: i32, tick_min: i32) -> *mut std::ffi::c_void;
    fn seeds_step(handle: *mut std::ffi::c_void, ticks: i32);
    fn seeds_get_alive_count(handle: *mut std::ffi::c_void) -> i32;
    fn seeds_destroy(handle: *mut std::ffi::c_void);
}
```

### Zig
```zig
const c = @cImport(@cInclude("seeds_engine/seeds_engine_c.h"));
const engine = c.seeds_create(42, 8, 8, 60);
defer c.seeds_destroy(engine);
c.seeds_step(engine, 10);
```

### Lua (5.5+ FFI)
```lua
local ffi = require("ffi")
ffi.cdef[[ /* paste seeds_engine_c.h declarations */ ]]
local lib = ffi.load("seeds_engine_c")
local engine = lib.seeds_create(42, 8, 8, 60)
lib.seeds_step(engine, 10)
lib.seeds_destroy(engine)
```

## MCP Server (AI integration)

The engine includes a JSON-RPC 2.0 MCP server for LLM tool-calling. Wire up handlers:

```cpp
#include <seeds_engine/net/mcp_server.hpp>
seeds::net::McpServer server;
server.set_handler("seeds.step", [&](auto& params) {
    int n = std::stoi(params.at("ticks"));
    engine.step(n);
    return "stepped " + std::to_string(n) + " ticks";
});
server.run();  // reads stdin, writes stdout (JSON-RPC 2.0)
```

Exposed tools: `seeds.step`, `seeds.spawn`, `seeds.snapshot`, `seeds.analytics`,
`seeds.disaster`, `seeds.get_tick`, `seeds.get_season`, `seeds.alive_count`,
`seeds.organism_info`, `seeds.set_belief`

## Debugger CLI

```
$ ./seeds_debugger --seed 42 --width 8 --height 8
Seeds Engine Debugger v1.0.0 (seed=42, world=8x8)
Type 'help' for commands.

(seeds-dbg) spawn herbivore 2 2 female 24
(seeds-dbg) spawn herbivore 2 2 male 24
(seeds-dbg) bp pop 1
(seeds-dbg) watch O000001
(seeds-dbg) step 100
*** BREAKPOINT HIT: population_below = 1 (id:1)
(seeds-dbg) organism O000001
(seeds-dbg) events 10
(seeds-dbg) flush
(seeds-dbg) quit
```

## Faith module

Built-in belief systems: `sun_worship`, `ancestor_spirits`, `nature_harmony`, `predator_cult`.

Each organism carries a `FaithState` with devotion, community bond, and conversion cooldown.
The engine updates faith per tick based on wellbeing and co-believers on the same tile.
Beliefs grant behavior modifiers: aggression reduction, cooperation, fertility, stress relief.

Register custom beliefs:
```cpp
engine.faith_engine().register_belief(life::BeliefSystem{
    .belief_id = "moon_cult", .name = "Lunar Order",
    .peace_bonus = 0.3, .cooperation_bonus = 0.2,
    .fertility_modifier = 0.0, .mortality_comfort = 0.3,
    .tenets = {"night is sacred", "howl at full moon"},
});
```

## Project structure

```
include/seeds_engine/
  core/                   # Engine loop, clock, events, scheduler
  world/                  # Map, biome, terrain, climate, resources
  life/                   # Species, genetics, organism, physiology, disease, aging, faith
  behavior/               # Actions, perception, utility AI, social
  ecology/                # Competition, foodweb, migration, population, predation
  storage/                # Snapshots, repository, formats
  ai/                     # Anomaly, clustering, prediction, reports
  debug/                  # Debugger (events, breakpoints, watch)
  net/                    # MCP server (JSON-RPC 2.0)
  api/                    # Facade, schemas
  export.hpp              # SEEDS_API dllexport/import macros
  seeds_engine.hpp        # Single-include C++ header
  seeds_engine_c.h        # C ABI header (extern "C")
src/
  seeds_engine.cpp        # C++ library implementation
  seeds_engine_c.cpp      # C ABI implementation
  main.cpp                # Demo application
  seeds_debugger.cpp      # Debugger tool
tests/
  test_backend_phase1.cpp # Test suite (8 tests)
CMakeLists.txt            # Build configuration (v1.0.0)
```

## License

MIT
Copyright © 2026 Neofilisoft / Chakrapong Boonpa
