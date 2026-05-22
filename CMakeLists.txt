cmake_minimum_required(VERSION 3.20)
project(seeds_engine VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# On Windows, place .dll next to executables for runtime discovery
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ================================================================
# Core shared (dynamic) library — C++ API
# ================================================================
add_library(seeds_engine SHARED src/seeds_engine.cpp)
target_include_directories(seeds_engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(seeds_engine PRIVATE SEEDS_ENGINE_BUILDING)

# ================================================================
# C ABI shared library — extern "C" FFI for C#, Rust, Zig, Lua, Java, JS
# ================================================================
option(SEEDS_BUILD_CAPI "Build the C ABI shared library" ON)
if(SEEDS_BUILD_CAPI)
    add_library(seeds_engine_c SHARED src/seeds_engine_c.cpp src/seeds_engine.cpp)
    target_include_directories(seeds_engine_c PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_compile_definitions(seeds_engine_c PRIVATE SEEDS_ENGINE_BUILDING)
endif()

# ================================================================
# Demo executable
# ================================================================
add_executable(seeds_demo src/main.cpp)
target_link_libraries(seeds_demo PRIVATE seeds_engine)

# ================================================================
# Debugger Tool (standalone runtime)
# ================================================================
option(SEEDS_BUILD_DEBUGGER "Build the standalone debugger tool" ON)
if(SEEDS_BUILD_DEBUGGER)
    add_executable(seeds_debugger src/seeds_debugger.cpp)
    target_link_libraries(seeds_debugger PRIVATE seeds_engine)
endif()

# ================================================================
# Tests
# ================================================================
option(SEEDS_BUILD_TESTS "Build tests" ON)
if(SEEDS_BUILD_TESTS)
    enable_testing()
    add_executable(seeds_tests tests/test_backend_phase1.cpp)
    target_link_libraries(seeds_tests PRIVATE seeds_engine)
    add_test(NAME seeds_backend_phase1 COMMAND seeds_tests)
endif()
