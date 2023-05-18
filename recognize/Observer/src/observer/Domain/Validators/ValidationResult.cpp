#include "ValidationResult.hpp"

namespace Observer {
    ValidationResult::ValidationResult() : valid(false) {}

    EventDescriptor& ValidationResult::GetEvent() & { return this->event; }

    void ValidationResult::SetValid(bool pValid) { this->valid = pValid; }

    void ValidationResult::AddMessages(std::vector<std::string>&& pMessages) {
        std::move(pMessages.begin(), pMessages.end(),
                  std::back_inserter(this->messages));
    }

    void ValidationResult::SetBlobs(std::vector<Blob>&& pBlobs) {
        this->event.SetBlobs(std::move(pBlobs));
    }

    std::vector<Blob>& ValidationResult::GetBlobs() & {
        return this->event.GetBlobs();
    }

    void ValidationResult::SetDetections(
        std::vector<AsyncInference::ImageDetections>&& pDetections) {
        this->event.SetDetections(std::move(pDetections));
    }

    std::vector<AsyncInference::ImageDetections>&
    ValidationResult::GetDetections() & {
        return this->event.GetDetections();
    }

    bool ValidationResult::IsValid() { return this->valid; }

    std::vector<std::string>& ValidationResult::GetMessages() & {
        return this->messages;
    }
}  // namespace Observer