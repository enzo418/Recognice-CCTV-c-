#pragma once

#include <string>
#include <vector>

#include "../../Blob/BlobDetector/Blob.hpp"
#include "../Event/Event.hpp"

namespace Observer {
    template <typename TFrame>
    struct ValidationResult {
       public:
        ValidationResult()
            : valid(false) {}  // for use as default inside template

        explicit ValidationResult(bool pResult) : valid(pResult) {}

        ValidationResult(bool pResult,
                         const std::vector<std::string>& pMessages)
            : valid(pResult), messages(std::move(pMessages)) {}

        ValidationResult(bool pResult,
                         const std::vector<std::string>& pMessages,
                         std::vector<Blob>&& blobs)
            : valid(pResult),
              messages(std::move(pMessages)),
              event(std::move(blobs)) {}

        Event& GetEvent() & { return this->event; }

        bool IsValid() { return this->valid; }

        std::vector<std::string>& GetMessages() & { return this->messages; }

        ValidationResult(ValidationResult&& r) = delete;
        ValidationResult(const ValidationResult& r)
            : valid(r.valid), messages(r.messages), event(r.event) {};

       private:
        bool valid = false;
        std::vector<std::string> messages {};
        Event event;
    };
}  // namespace Observer