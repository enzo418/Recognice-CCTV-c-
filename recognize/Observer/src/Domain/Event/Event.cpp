#include "Event.hpp"

#include <utility>

namespace Observer {
    ClassifierFindingEvent::ClassifierFindingEvent(int pFrameIndex,
                                                   std::vector<Point> pPoints)
        : findingFrameIndex(pFrameIndex), points(std::move(pPoints)) {}

    int ClassifierFindingEvent::GetFindingIndex() {
        return this->findingFrameIndex;
    }

    std::vector<Point>& ClassifierFindingEvent::GetObjectPoints() & {
        return this->points;
    }

    Event::Event() = default;

    Event::Event(std::vector<Blob>&& blobs) : blobsFound(std::move(blobs)) {}

    void Event::SetBlobs(std::vector<Blob>&& findings) {
        this->blobsFound = std::move(findings);
    }

    void Event::AddClassifierFinding(ClassifierFindingEvent&& finding) {
        this->classifierFindings.push_back(std::move(finding));
    }

    void Event::SetCameraName(std::string pCameraName) {
        this->cameraName = pCameraName;
    }

    std::string Event::GetCameraName() { return this->cameraName; }

    std::vector<Blob>& Event::GetBlobs() { return this->blobsFound; }

    int Event::GetFirstFrameWhereFindingWasFound() {
        const int max_int = std::numeric_limits<int>::max();

        int index = max_int;

        // Since findings are processed from frame 0 to the last one
        // we just check the first of both results

        for (auto& blob : this->blobsFound) {
            if (blob.GetFirstAppearance() < index) {
                index = blob.GetFirstAppearance();
            }
        }

        if (!this->classifierFindings.empty()) {
            int index2 = this->classifierFindings.front().GetFindingIndex();

            if (index2 < index) {
                index = index2;
            }
        }

        return index == max_int ? 0 : index;
    }
};  // namespace Observer