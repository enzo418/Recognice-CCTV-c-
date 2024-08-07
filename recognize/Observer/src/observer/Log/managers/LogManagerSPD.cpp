#include "LogManagerSPD.hpp"

#include <spdlog/common.h>

#include "spdlog/sinks/rotating_file_sink.h"

namespace Observer {
    std::shared_ptr<spdlog::logger> LogManager::logger;

    void LogManager::Initialize() {
        // console sink
        auto console_sink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        console_sink->set_level(spdlog::level::trace);

        console_sink->set_pattern("%^[%l] %T %D [thread %t] %v%$");

        // file sink for errors
        auto max_size = 1024 * 1024 * 5;
        auto max_files = 3;
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/observer_log.txt", max_size, max_files);

        file_sink->set_level(spdlog::level::trace);

        // create logger
        std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};
        logger = std::make_shared<spdlog::logger>(OBSERVER_DEFAULT_LOGGER_NAME,
                                                  sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::trace);

        // register logger
        spdlog::register_logger(logger);
    }

    void LogManager::Shutdown() { spdlog::shutdown(); }
}  // namespace Observer