#pragma once

#include <string>
#include <vector>

#include "../../Blob/BlobDetector/Blob.hpp"
#include "../../IFrame.hpp"
#include "../Event/Event.hpp"

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

        Event& GetEvent() &;

        bool IsValid();

        std::vector<std::string>& GetMessages() &;

        ValidationResult(const ValidationResult& r);

       private:
        bool valid = false;
        std::vector<std::string> messages;
        Event event;
    };
}  // namespace Observer