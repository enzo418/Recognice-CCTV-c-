#pragma once

#include "observer/AsyncInference/types.hpp"
#include "observer/Blob/BlobDetector/Blob.hpp"

namespace Observer {
    struct BlobClassification {
        std::string label;
        float confidence;
        double IoU;
    };

    typedef int BlobId;
    typedef std::unordered_map<BlobId, BlobClassification> BlobClassifications;

    /**
     * @brief Assigns a class to each blob.
     *
     * @param blobs
     * @param sequenceDetections
     * @return BlobClassifications
     */
    BlobClassifications AssignObjectToBlob(
        std::vector<Blob>& blobs,
        std::vector<AsyncInference::ImageDetections>& sequenceDetections);
}  // namespace Observer