/**
 * Seeds Engine v1.0.0 — C ABI (extern "C") Interface
 *
 * This header provides a flat C API suitable for FFI consumption from:
 *   - C# (.NET 10+) via P/Invoke or LibraryImport
 *   - Rust (1.95+) via extern "C" FFI
 *   - Zig via @cImport
 *   - Lua (5.5+) via FFI or C module
 *   - Java via JNI or Panama (Project Panama)
 *   - JavaScript/WASM via Emscripten or wasm-bindgen helper
 *
 * All strings returned are owned by the engine and valid until the next
 * mutating call on the same handle. Caller must NOT free them.
 *
 * Thread safety: One handle must only be used from one thread at a time.
 */

#ifndef SEEDS_ENGINE_C_H
#define SEEDS_ENGINE_C_H

#include <stdint.h>

#ifdef _WIN32
    #ifdef SEEDS_ENGINE_BUILDING
        #define SEEDS_CAPI __declspec(dllexport)
    #else
        #define SEEDS_CAPI __declspec(dllimport)
    #endif
#else
    #ifdef SEEDS_ENGINE_BUILDING
        #define SEEDS_CAPI __attribute__((visibility("default")))
    #else
        #define SEEDS_CAPI
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handle to a Seeds Engine instance */
typedef struct SeedsEngine_t* SeedsEngineHandle;

/* ================================================================
 * Lifecycle
 * ================================================================ */

/** Create a new engine with default Phase 1 ecology (herbivore + carnivore).
 *  @param seed          RNG seed for determinism
 *  @param width         World grid width
 *  @param height        World grid height
 *  @param tick_minutes  Simulated minutes per tick (e.g. 60)
 *  @return              Opaque engine handle (NULL on failure)
 */
SEEDS_CAPI SeedsEngineHandle seeds_create(int seed, int width, int height, int tick_minutes);

/** Destroy an engine instance and free all associated memory. */
SEEDS_CAPI void seeds_destroy(SeedsEngineHandle handle);

/* ================================================================
 * Species Registration
 * ================================================================ */

/** Register a built-in herbivore species profile.
 *  @param handle      Engine handle
 *  @param species_id  Unique ID string for this species
 *  @param name        Display name
 */
SEEDS_CAPI void seeds_register_herbivore(SeedsEngineHandle handle, const char* species_id, const char* name);

/** Register a built-in carnivore species profile. */
SEEDS_CAPI void seeds_register_carnivore(SeedsEngineHandle handle, const char* species_id, const char* name);

/* ================================================================
 * Spawning
 * ================================================================ */

/** Spawn an organism at (x, y).
 *  @param sex  "male", "female", or "" for random
 *  @param age_ticks  Initial age in ticks
 *  @return  Organism ID string (engine-owned, valid until next mutation)
 */
SEEDS_CAPI const char* seeds_spawn(SeedsEngineHandle handle,
                                   const char* species_id,
                                   int x, int y,
                                   const char* sex,
                                   int age_ticks);

/* ================================================================
 * Simulation Control
 * ================================================================ */

/** Advance the simulation by `ticks` steps. */
SEEDS_CAPI void seeds_step(SeedsEngineHandle handle, int ticks);

/** Schedule a disaster event.
 *  @param tick      Tick at which the disaster fires
 *  @param x, y     Epicenter coordinates
 *  @param radius   Manhattan-distance radius
 *  @param severity 0.0 - 1.0
 */
SEEDS_CAPI void seeds_schedule_disaster(SeedsEngineHandle handle,
                                        int tick, int x, int y,
                                        int radius, double severity);

/* ================================================================
 * Queries (read-only)
 * ================================================================ */

/** Get current simulation tick. */
SEEDS_CAPI int seeds_get_tick(SeedsEngineHandle handle);

/** Get current season string ("spring", "summer", "autumn", "winter"). */
SEEDS_CAPI const char* seeds_get_season(SeedsEngineHandle handle);

/** Get count of currently alive organisms. */
SEEDS_CAPI int seeds_get_alive_count(SeedsEngineHandle handle);

/** Get alive count for a specific species. Returns 0 if unknown. */
SEEDS_CAPI int seeds_get_species_count(SeedsEngineHandle handle, const char* species_id);

/** Get a JSON snapshot of the full world state.
 *  Caller must free the returned string with seeds_free_string(). */
SEEDS_CAPI char* seeds_snapshot_json(SeedsEngineHandle handle);

/** Get a JSON analytics report.
 *  Caller must free the returned string with seeds_free_string(). */
SEEDS_CAPI char* seeds_analytics_json(SeedsEngineHandle handle);

/* ================================================================
 * Organism Query
 * ================================================================ */

/** Get organism health (0.0 - 1.0). Returns -1.0 if not found. */
SEEDS_CAPI double seeds_organism_health(SeedsEngineHandle handle, const char* organism_id);

/** Get organism energy (0.0 - 1.0). Returns -1.0 if not found. */
SEEDS_CAPI double seeds_organism_energy(SeedsEngineHandle handle, const char* organism_id);

/** Check if organism is alive. Returns 0 (false) or 1 (true), -1 if not found. */
SEEDS_CAPI int seeds_organism_alive(SeedsEngineHandle handle, const char* organism_id);

/** Get organism position. Writes into *out_x, *out_y. Returns 0 on success, -1 on failure. */
SEEDS_CAPI int seeds_organism_position(SeedsEngineHandle handle, const char* organism_id,
                                       int* out_x, int* out_y);

/* ================================================================
 * Faith Module (v1.0.0)
 * ================================================================ */

/** Assign a belief system to an organism.
 *  @param belief_id  e.g. "sun_worship", "ancestor_spirits"
 *  @param devotion   Initial devotion level (0.0 - 1.0)
 */
SEEDS_CAPI void seeds_set_belief(SeedsEngineHandle handle,
                                 const char* organism_id,
                                 const char* belief_id,
                                 double devotion);

/** Get the dominant belief of an organism. Returns "" if none. */
SEEDS_CAPI const char* seeds_get_belief(SeedsEngineHandle handle, const char* organism_id);

/** Get devotion level of an organism. Returns -1.0 if not found. */
SEEDS_CAPI double seeds_get_devotion(SeedsEngineHandle handle, const char* organism_id);

/* ================================================================
 * Debugger (v1.0.0)
 * ================================================================ */

/** Enable tick-by-tick event logging to internal buffer. */
SEEDS_CAPI void seeds_debug_enable(SeedsEngineHandle handle);

/** Disable debug logging. */
SEEDS_CAPI void seeds_debug_disable(SeedsEngineHandle handle);

/** Get the debug log as a JSON array of events since last call.
 *  Caller must free with seeds_free_string(). Clears the internal buffer. */
SEEDS_CAPI char* seeds_debug_flush(SeedsEngineHandle handle);

/* ================================================================
 * Memory Management
 * ================================================================ */

/** Free a string allocated by the C API (snapshot_json, analytics_json, debug_flush). */
SEEDS_CAPI void seeds_free_string(char* str);

/* ================================================================
 * Version
 * ================================================================ */

/** Returns version string, e.g. "1.0.0". Engine-owned, do not free. */
SEEDS_CAPI const char* seeds_version(void);

#ifdef __cplusplus
}
#endif

#endif /* SEEDS_ENGINE_C_H */
