#pragma once

#include <string>
#include <utility>
#include <vector>

#include "observer/Blob/BlobDetector/Blob.hpp"
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

    class Event {
       public:
        Event();

        Event(std::vector<Blob>&& blobs);

        void SetBlobs(std::vector<Blob>&& findings);
        std::vector<Blob>& GetBlobs();

        void AddClassifierFinding(ClassifierFindingEvent&& finding);

        void SetCameraName(std::string cameraName);

        std::string GetCameraName();

        int GetFirstFrameWhereFindingWasFound();

       private:
        // camera
        std::string cameraName;

        // frames/findings
        std::vector<Blob> blobsFound;
        std::vector<ClassifierFindingEvent> classifierFindings;
    };

}  // namespace Observer
