#pragma once

#ifdef _WIN32
    #ifdef SEEDS_ENGINE_BUILDING
        #define SEEDS_API __declspec(dllexport)
    #else
        #define SEEDS_API __declspec(dllimport)
    #endif
#else
    #ifdef SEEDS_ENGINE_BUILDING
        #define SEEDS_API __attribute__((visibility("default")))
    #else
        #define SEEDS_API
    #endif
#endif
