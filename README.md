# Seeds Engine (Simulation Ecology Entity Dynamic System) - C++20

Seeds Engine is a backend-ready, header-only C++20 simulation library for sandbox simulations and game embedding.
It focuses on deterministic, seeded ecology simulation with extensible subsystems for world state, organisms, genetics, physiology, behavior, ecology, storage, and analytics.

## Included backend layers

- `core/`: simulation clock, scheduler, events, deterministic engine loop
- `world/`: grid map, chunking, biome, terrain, climate, resources
- `life/`: species, organisms, genetics, physiology, disease, reproduction, aging
- `behavior/`: perception and utility-based decision selection
- `ecology/`: predation, migration, competition, population summaries, foodweb rules
- `storage/`: snapshots and repository helpers
- `ai/`: optional analytics helpers layered on top of the simulation
- `api/`: facade and schemas for embedding the engine into a game or service

## Requirements

- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- CMake 3.20+

## Time model

Default Seeds timing:
- `1 tick = 60 simulated minutes`
- Change to `TimeConfig{.tick_minutes = 10}` for `1 tick = 10 simulated minutes`
- Engine execution remains seeded and deterministic through `seed` configuration

## Build & Run

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/seeds_demo --ticks 24
./build/seeds_tests
```

On Windows (MSVC):
```powershell
cmake -B build
cmake --build build --config Release
.\build\Release\seeds_demo.exe --ticks 24
.\build\Release\seeds_tests.exe
```

## Minimal embedding example

```cpp
#include <seeds_engine/seeds_engine.hpp>

using namespace seeds;

int main() {
    core::TimeConfig time_config{.tick_minutes = 60};
    api::WorldConfig world_config{.width = 8, .height = 8, .chunk_size = 4,
                                  .default_biome = world::BiomeType::Grassland};

    auto facade = api::create_phase1_engine(42, time_config, world_config);

    facade.spawn(api::SpawnRequest{.species_id = "herbivore", .x = 2, .y = 2, .sex = "female", .age_ticks = 24});
    facade.spawn(api::SpawnRequest{.species_id = "herbivore", .x = 2, .y = 2, .sex = "male", .age_ticks = 24});
    facade.spawn(api::SpawnRequest{.species_id = "carnivore", .x = 5, .y = 5, .age_ticks = 24});

    facade.step(10);
    auto snapshot = facade.snapshot();
    auto report = facade.analytics_report();
    return 0;
}
```

## Project structure

```
include/seeds_engine/     # Header-only library
  core/                   # Engine loop, clock, events, scheduler
  world/                  # Map, biome, terrain, climate, resources
  life/                   # Organism, species, genetics, physiology, disease, reproduction, aging
  behavior/               # Actions, perception, utility AI, social
  ecology/                # Competition, foodweb, migration, population, predation
  storage/                # Snapshots, repository, formats
  ai/                     # Anomaly detection, clustering, prediction, reports
  api/                    # Facade, schemas
  seeds_engine.hpp        # Single-include convenience header
src/main.cpp              # Demo application
tests/                    # Test suite
CMakeLists.txt            # Build configuration
```

Copyright © 2026 Neofilisoft / Chakrapong Boonpa
