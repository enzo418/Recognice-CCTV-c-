#include "BlobClassification.hpp"

namespace Observer {
    BlobClassifications AssignObjectToBlob(
        std::vector<Blob>& blobs,
        std::vector<AsyncInference::ImageDetections>& sequenceDetections) {
        BlobClassifications blobToClassProb;

        if (blobs.empty() || sequenceDetections.empty()) {
            return blobToClassProb;
        }

        for (auto& imageDetections : sequenceDetections) {
            auto& detections = imageDetections.detections;
            int index = imageDetections.image_index;

            for (auto& detection : detections) {  // avg is about < 4 elements
                double maxIoU = 0;
                Blob* maxIoUBlob = nullptr;
                Rect rectDetection = Rect(detection.x, detection.y,
                                          detection.width, detection.height);

                for (auto& blob : blobs) {  // avg about < 4 blobs
                    if (blob.GetFirstAppearance() <= index &&
                        blob.GetLastAppearance() >= index) {
                        const int id = blob.GetId();

                        Rect blobRect = blob.GetBoundingRect(index);

                        float IoU =
                            blobRect.IntersectionOverUnion(rectDetection);

                        if (IoU > maxIoU && (!blobToClassProb.contains(id) ||
                                             blobToClassProb[id].IoU < IoU)) {
                            maxIoU = IoU;
                            maxIoUBlob = &blob;
                        }
                    }
                }

                if (maxIoUBlob != nullptr) {
                    const int id = maxIoUBlob->GetId();
                    if (blobToClassProb.contains(id)) {
                        blobToClassProb[id] = BlobClassification {
                            .label = detection.label,
                            .confidence = detection.confidence,
                            .IoU = maxIoU,
                        };
                    } else {
                        blobToClassProb.insert(
                            {id, BlobClassification {
                                     .label = detection.label,
                                     .confidence = detection.confidence,
                                     .IoU = maxIoU,
                                 }});
                    }
                }
            }
        }

        return blobToClassProb;
    }

}  // namespace Observer