#pragma once

#include "Event.hpp"

namespace Observer {
    StandarFindingEvent::StandarFindingEvent(int pFrameIndex, cv::Point pCenter, cv::Rect pRect)  :
            findingFrameIndex(pFrameIndex),
            center(std::move(pCenter)),
            rect(std::move(pRect)) {

    }

    ClassifierFindingEvent::ClassifierFindingEvent(int pFrameIndex, std::vector<cv::Point> pPoints) :
            findingFrameIndex(pFrameIndex),
            points(std::move(pPoints)) {}
};