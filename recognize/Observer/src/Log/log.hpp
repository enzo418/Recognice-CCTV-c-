#pragma once

// use spdlog implementation
#include "managers/LogManagerSPD.hpp"

#ifndef OBSERVER_RELEASE
#define OBSERVER_TRACE(...) \
    Observer::LogManager::GetLogger()->trace(__VA_ARGS__)
#define OBSERVER_INFO(...) \
    Observer::LogManager::GetLogger()->info(__VA_ARGS__)
#define OBSERVER_WARN(...) \
    Observer::LogManager::GetLogger()->warn(__VA_ARGS__)
#define OBSERVER_ERROR(...) \
    Observer::LogManager::GetLogger()->error(__VA_ARGS__)
#define OBSERVER_CRITICAL(...) \
    Observer::LogManager::GetLogger()->critical(__VA_ARGS__)
#else
#define
#define OBSERVER_TRACE(...) \
    Observer::LogManager::GetLogger()->trace(__VA_ARGS__)
#define OBSERVER_INFO(...) \
    Observer::LogManager::GetLogger()->info(__VA_ARGS__)
#define OBSERVER_WARN(...) \
    Observer::LogManager::GetLogger()->warn(__VA_ARGS__)
#define OBSERVER_ERROR(...) \
    Observer::LogManager::GetLogger()->error(__VA_ARGS__)
#define OBSERVER_CRITICAL(...) \
    Observer::LogManager::GetLogger()->critical(__VA_ARGS__)
#endif
