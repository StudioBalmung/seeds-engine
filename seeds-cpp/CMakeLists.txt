cmake_minimum_required(VERSION 3.20)
project(seeds_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only library
add_library(seeds_engine INTERFACE)
target_include_directories(seeds_engine INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Demo executable
add_executable(seeds_demo src/main.cpp)
target_link_libraries(seeds_demo PRIVATE seeds_engine)

# Tests
option(SEEDS_BUILD_TESTS "Build tests" ON)
if(SEEDS_BUILD_TESTS)
    enable_testing()
    add_executable(seeds_tests tests/test_backend_phase1.cpp)
    target_link_libraries(seeds_tests PRIVATE seeds_engine)
    add_test(NAME seeds_backend_phase1 COMMAND seeds_tests)
endif()
