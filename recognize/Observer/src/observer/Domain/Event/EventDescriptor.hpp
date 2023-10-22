#pragma once

#include <string>
#include <utility>
#include <vector>

#include "observer/AsyncInference/types.hpp"
#include "observer/Blob/BlobDetector/Blob.hpp"
#include "observer/Domain/Classification/BlobClassification.hpp"
#include "observer/Point.hpp"
#include "observer/Rect.hpp"

namespace Observer {
    class ClassifierFindingEvent {
       public:
        ClassifierFindingEvent(int pFrameIndex, std::vector<Point> pPoints);

        int GetFindingIndex();

        std::vector<Point>& GetObjectPoints() &;

       private:
        int findingFrameIndex;
        std::vector<Point> points;
    };

    class EventDescriptor {
       public:
        EventDescriptor();

        EventDescriptor(std::vector<Blob>&& blobs);
        EventDescriptor(
            std::vector<Blob>&& blobs,
            std::vector<AsyncInference::ImageDetections>&& detections);

        /* ------------------------ Blobs ----------------------- */
        void SetBlobs(std::vector<Blob>&& findings);
        std::vector<Blob>& GetBlobs();

        /* ---------------------- Detections -------------------- */
        void SetDetections(
            std::vector<AsyncInference::ImageDetections>&& detections);
        std::vector<AsyncInference::ImageDetections>& GetDetections();

        /* --------------------- Classifications ---------------- */
        void SetClassifications(BlobClassifications&& classifications);
        BlobClassifications& GetClassifications();

        /* --------------------- Camera Name -------------------- */
        void SetCameraName(const std::string& cameraName);
        std::string GetCameraName();

        void AddClassifierFinding(ClassifierFindingEvent&& finding);

        int GetFirstFrameWhereFindingWasFound();

       private:
        // camera
        std::string cameraName;

        // frames/findings
        std::vector<Blob> blobsFound;
        std::vector<AsyncInference::ImageDetections> detections;
        std::vector<ClassifierFindingEvent> classifierFindings;
        BlobClassifications classifications;
    };

}  // namespace Observer
