#pragma once

#include "observer/Domain/ObserverCentral.hpp"

namespace Web {
    /**
     * @brief Holds the context for the detection program.
     */
    struct RecognizeContext {
        /**
         * @brief Indicates if the detection program is running.
         */
        bool running;

        /**
         * @brief Entry point for the detection program.
         * @attention Accessing this field while `running` is false will
         * segfault.
         */
        std::unique_ptr<Observer::ObserverCentral> observer;

        /**
         * @brief The ID of the running configuration.
         */
        std::string running_config_id;
    };
};  // namespace Web