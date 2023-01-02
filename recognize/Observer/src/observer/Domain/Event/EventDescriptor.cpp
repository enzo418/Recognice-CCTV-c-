#include "EventDescriptor.hpp"

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

    EventDescriptor::EventDescriptor() = default;

    EventDescriptor::EventDescriptor(std::vector<Blob>&& blobs)
        : blobsFound(std::move(blobs)) {}

    void EventDescriptor::SetBlobs(std::vector<Blob>&& findings) {
        this->blobsFound = std::move(findings);
    }

    void EventDescriptor::AddClassifierFinding(
        ClassifierFindingEvent&& finding) {
        this->classifierFindings.push_back(std::move(finding));
    }

    void EventDescriptor::SetCameraName(std::string pCameraName) {
        this->cameraName = pCameraName;
    }

    std::string EventDescriptor::GetCameraName() { return this->cameraName; }

    std::vector<Blob>& EventDescriptor::GetBlobs() { return this->blobsFound; }

    int EventDescriptor::GetFirstFrameWhereFindingWasFound() {
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