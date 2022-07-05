#include "ValidationResult.hpp"

namespace Observer {
    ValidationResult::ValidationResult()
        : valid(false) {}  // for use as default inside template

    ValidationResult::ValidationResult(bool pResult) : valid(pResult) {}

    ValidationResult::ValidationResult(
        bool pResult, const std::vector<std::string>& pMessages)
        : valid(pResult), messages(std::move(pMessages)) {}

    ValidationResult::ValidationResult(
        bool pResult, const std::vector<std::string>& pMessages,
        std::vector<Blob>&& blobs)
        : valid(pResult),
          messages(std::move(pMessages)),
          event(std::move(blobs)) {}

    ValidationResult::ValidationResult(const ValidationResult& r)
        : valid(r.valid), messages(r.messages), event(r.event) {};

    Event& ValidationResult::GetEvent() & { return this->event; }

    bool ValidationResult::IsValid() { return this->valid; }

    std::vector<std::string>& ValidationResult::GetMessages() & {
        return this->messages;
    }
}  // namespace Observer