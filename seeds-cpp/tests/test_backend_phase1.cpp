#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

#include <seeds_engine/seeds_engine.hpp>

using namespace seeds;

static int tests_passed = 0;
static int tests_total = 0;

#define TEST(name) \
    void name(); \
    struct name##_register { name##_register() { tests_total++; } } name##_instance; \
    void name()

#define RUN_TEST(name) \
    do { \
        std::cout << "  Running " #name "..." << std::flush; \
        name(); \
        tests_passed++; \
        std::cout << " PASSED\n"; \
    } while(0)

#define ASSERT(cond) \
    do { if (!(cond)) { std::cerr << "\n  FAIL: " #cond " at line " << __LINE__ << "\n"; std::abort(); } } while(0)

void test_time_config_supports_hour_and_ten_minute_ticks() {
    core::TimeConfig config;
    config.tick_minutes = 10;
    assert(config.seconds_per_tick() == 600);

    auto facade = api::create_phase1_engine(7, config);
    facade.step(6);

    ASSERT(facade.engine().clock.tick() == 6);
    ASSERT(std::abs(facade.engine().clock.current_time_hours() - 1.0) < 1e-9);
}

void test_phase1_engine_runs_and_snapshot_contains_world_state() {
    api::WorldConfig wc;
    wc.width = 4;
    wc.height = 4;
    wc.chunk_size = 2;
    wc.default_biome = world::BiomeType::Grassland;

    auto facade = api::create_phase1_engine(11, core::TimeConfig{}, wc);
    facade.spawn(api::SpawnRequest{.species_id = "herbivore", .x = 1, .y = 1, .age_ticks = 24});
    facade.step(3);

    auto snap = facade.snapshot();
    ASSERT(std::get<int>(snap["tick"]) == 3);
    ASSERT(std::get<int>(snap["total_alive"]) >= 1);
    auto& resources = std::get<std::map<std::string, double>>(snap["world_resources"]);
    ASSERT(resources["plants"] > 0);
}

void test_reproduction_is_scheduled_and_creates_offspring() {
    auto facade = api::create_phase1_engine(13);
    auto& female = facade.spawn(api::SpawnRequest{.species_id = "herbivore", .x = 2, .y = 2, .sex = "female", .age_ticks = 24});
    auto& male = facade.spawn(api::SpawnRequest{.species_id = "herbivore", .x = 2, .y = 2, .sex = "male", .age_ticks = 24});

    facade.step(2);

    int alive_count = 0;
    for (const auto& [id, org] : facade.engine().organisms) {
        if (org.alive) alive_count++;
    }
    ASSERT(alive_count >= 3);
    ASSERT(female.offspring_count >= 1);
    ASSERT(male.offspring_count >= 1);
}

void test_disaster_event_hits_local_resources() {
    auto facade = api::create_phase1_engine(19);
    auto& tile = facade.engine().world.get_tile(0, 0);
    double before = tile.resources.plants;

    facade.engine().schedule_disaster(1, {0, 0}, 0, 0.5);
    facade.step(1);

    ASSERT(tile.resources.plants < before);
}

void test_analytics_report_stays_helper_layer() {
    auto facade = api::create_phase1_engine(23);
    facade.spawn(api::SpawnRequest{.species_id = "herbivore", .x = 0, .y = 0, .age_ticks = 24});
    facade.step(1);

    auto report = facade.analytics_report();
    ASSERT(report.count("carrying_capacity"));
    ASSERT(report.count("anomalies"));
    ASSERT(report.count("species_clusters"));
}

int main() {
    std::cout << "Seeds Engine C++20 - Backend Phase 1 Tests\n";
    std::cout << "==========================================\n";

    RUN_TEST(test_time_config_supports_hour_and_ten_minute_ticks);
    RUN_TEST(test_phase1_engine_runs_and_snapshot_contains_world_state);
    RUN_TEST(test_reproduction_is_scheduled_and_creates_offspring);
    RUN_TEST(test_disaster_event_hits_local_resources);
    RUN_TEST(test_analytics_report_stays_helper_layer);

    std::cout << "\n" << tests_passed << "/" << 5 << " tests passed.\n";
    return (tests_passed == 5) ? 0 : 1;
}
