#pragma once

#include <string>
#include <vector>

#include "observer/Blob/BlobDetector/Blob.hpp"
#include "observer/Domain/Event/EventDescriptor.hpp"
#include "observer/IFrame.hpp"

namespace Observer {
    struct ValidationResult {
       public:
        ValidationResult();

        explicit ValidationResult(bool pResult);

        ValidationResult(bool pResult,
                         const std::vector<std::string>& pMessages);

        ValidationResult(bool pResult,
                         const std::vector<std::string>& pMessages,
                         std::vector<Blob>&& blobs);

        EventDescriptor& GetEvent() &;

        bool IsValid();

        std::vector<std::string>& GetMessages() &;

        ValidationResult(const ValidationResult& r);

       private:
        bool valid = false;
        std::vector<std::string> messages;
        EventDescriptor event;
    };
}  // namespace Observer