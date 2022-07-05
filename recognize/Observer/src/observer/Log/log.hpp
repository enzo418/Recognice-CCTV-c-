#pragma once

// use spdlog implementation
#include "managers/LogManagerSPD.hpp"

#ifdef __MINGW32__
#define OBSERVER_BREAK __debugbreak();
#elif defined(__APPLE__)
#define OBSERVER_BREAK __builtin_debugtrap();
#elif defined(__linux__)
#define OBSERVER_BREAK __builtin_trap();
#else

#endif

#ifndef OBSERVER_RELEASE
#define OBSERVER_TRACE(...) \
    Observer::LogManager::GetLogger()->trace(__VA_ARGS__)
#define OBSERVER_INFO(...) Observer::LogManager::GetLogger()->info(__VA_ARGS__)
#define OBSERVER_WARN(...) Observer::LogManager::GetLogger()->warn(__VA_ARGS__)
#define OBSERVER_ERROR(...) \
    Observer::LogManager::GetLogger()->error(__VA_ARGS__)
#define OBSERVER_CRITICAL(...) \
    Observer::LogManager::GetLogger()->critical(__VA_ARGS__)
#define OBSERVER_ASSERT(x, msg)                                            \
    if ((x)) {                                                             \
    } else {                                                               \
        OBSERVER_CRITICAL("ASSERT FAILED - {}, \n\tFile: {}\n\tLine: {} ", \
                          msg, __FILE__, __LINE__);                        \
        OBSERVER_BREAK                                                     \
    }
#else
#define
#define OBSERVER_TRACE(...) (void)0
#define OBSERVER_INFO(...) (void)0
#define OBSERVER_WARN(...) Observer::LogManager::GetLogger()->warn(__VA_ARGS__)
#define OBSERVER_ERROR(...) \
    Observer::LogManager::GetLogger()->error(__VA_ARGS__)
#define OBSERVER_CRITICAL(...) \
    Observer::LogManager::GetLogger()->critical(__VA_ARGS__)
#define OBSERVER_ASSERT(x, msg) (void)0
#endif
