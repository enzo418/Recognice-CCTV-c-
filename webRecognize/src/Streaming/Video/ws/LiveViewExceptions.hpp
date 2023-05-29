#pragma once

#include <stdexcept>

namespace Web::Streaming::Video::Ws {
    class InvalidCameraUriException : virtual public std::runtime_error {
       public:
        InvalidCameraUriException(const std::string& msg = "")
            : std::runtime_error(msg) {}

        /**
         * @brief Destroy the Invalid Camera Uri Exception object
         * Virtual to allow for subclassing
         */
        virtual ~InvalidCameraUriException() throw() {}
    };

    class ObserverNotRunningException : virtual public std::runtime_error {
       public:
        ObserverNotRunningException(const std::string& msg = "")
            : std::runtime_error(msg) {}

        /**
         * @brief Destroy the Invalid Camera Uri Exception object
         * Virtual to allow for subclassing
         */
        virtual ~ObserverNotRunningException() throw() {}
    };
}  // namespace Web::Streaming::Video::Ws