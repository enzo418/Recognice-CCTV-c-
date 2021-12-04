#pragma once

#include "log_constants.hpp"

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <memory>
#include <vector>

namespace Observer {
    class LogManager {
       public:
        static void Initialize();
        static void Shutdown();

        static std::shared_ptr<spdlog::logger> GetLogger() {return logger;}

       private:
        static std::shared_ptr<spdlog::logger> logger;
        
        LogManager() = default;
        ~LogManager() = default;
    };
}