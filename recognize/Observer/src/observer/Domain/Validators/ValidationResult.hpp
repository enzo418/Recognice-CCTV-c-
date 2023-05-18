#pragma once

#include <string>
#include <vector>

#include "observer/AsyncInference/types.hpp"
#include "observer/Blob/BlobDetector/Blob.hpp"
#include "observer/Domain/Event/EventDescriptor.hpp"
#include "observer/IFrame.hpp"

namespace Observer {
    struct ValidationResult {
       public:
        ValidationResult();

        /* ----------------------- Common ----------------------- */
        EventDescriptor& GetEvent() &;

        /* --------------------- Valid Event -------------------- */
        void SetValid(bool pValid);
        bool IsValid();

        /* --------------------- Messages ----------------------- */
        void AddMessages(std::vector<std::string>&& pMessages);
        std::vector<std::string>& GetMessages() &;

        /* ----------------------- Blobs ------------------------ */
        void SetBlobs(std::vector<Blob>&& pBlobs);
        std::vector<Blob>& GetBlobs() &;

        /* --------------------- Detections --------------------- */
        void SetDetections(
            std::vector<AsyncInference::ImageDetections>&& pDetections);

        std::vector<AsyncInference::ImageDetections>& GetDetections() &;

       private:
        bool valid = false;
        std::vector<std::string> messages;
        EventDescriptor event;
    };
}  // namespace Observer