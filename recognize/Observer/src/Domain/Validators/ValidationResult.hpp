#pragma once

#include <string>
#include <vector>

#include "../Event/Event.hpp"

namespace Observer {
    template <typename TFrame>
    struct ValidationResult {
       public:
        ValidationResult()
            : valid(false) {}  // for use as default inside template

        explicit ValidationResult(bool pResult) : valid(pResult) {}

        ValidationResult(bool pResult, std::vector<std::string>& pMessages)
            : valid(pResult), messages(pMessages) {}

        Event& GetEvent() & { return this->event; }

        void SetValid(bool pValid) { this->valid = pValid; }

        bool IsValid() { return this->valid; }

        void AddMessage(std::string&& message) {
            this->messages.push_back(std::move(message));
        }

        std::vector<std::string>& GetMessages() & { return this->messages; }

       private:
        bool valid = false;
        std::vector<std::string> messages {};
        Event event;
    };
}  // namespace Observer