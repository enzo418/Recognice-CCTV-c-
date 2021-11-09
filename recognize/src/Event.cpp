#include "Event.hpp"

#include <utility>

namespace Observer {
    StandarFindingEvent::StandarFindingEvent(int pFrameIndex, cv::Point pCenter,
                                             cv::Rect pRect)
        : findingFrameIndex(pFrameIndex),
          center(std::move(pCenter)),
          rect(std::move(pRect)) {}

    int StandarFindingEvent::GetFindingIndex() {
        return this->findingFrameIndex;
    }

    cv::Point StandarFindingEvent::GetCenter() { return this->center; }

    cv::Rect StandarFindingEvent::GetRect() { return this->rect; }

    ClassifierFindingEvent::ClassifierFindingEvent(
        int pFrameIndex, std::vector<cv::Point> pPoints)
        : findingFrameIndex(pFrameIndex), points(std::move(pPoints)) {}

    int ClassifierFindingEvent::GetFindingIndex() {
        return this->findingFrameIndex;
    }

    std::vector<cv::Point>& ClassifierFindingEvent::GetObjectPoints() & {
        return this->points;
    }

    Event::Event() = default;

    void Event::AddStandarFinding(StandarFindingEvent&& finding) {
        this->standarFindings.push_back(std::move(finding));
    }

    void Event::AddClassifierFinding(ClassifierFindingEvent&& finding) {
        this->classifierFindings.push_back(std::move(finding));
    }

    void Event::SetCameraName(std::string pCameraName) {
        this->cameraName = std::move(pCameraName);
    }

    std::string Event::GetCameraName() { return this->cameraName; }

    int Event::GetFirstFrameWhereFindingWasFound() {
        int index = 0;

        // Since findings are processed from frame 0 to the last one
        // we just check the first of both results

        if (!this->standarFindings.empty()) {
            index = this->standarFindings.front().GetFindingIndex();
        }

        if (!this->classifierFindings.empty()) {
            int index2 = this->classifierFindings.front().GetFindingIndex();

            if (index2 < index) {
                index = index2;
            }
        }

        return index;
    }
};  // namespace Observer