#pragma once

#include <opencv2/opencv.hpp>
#include <utility>

namespace Observer
{
    class StandarFindingEvent {
        public:
            StandarFindingEvent(int pFrameIndex, cv::Point pCenter, cv::Rect pRect);
        private:
            int findingFrameIndex;
            cv::Point center;
            cv::Rect rect;
    };

    class ClassifierFindingEvent {
        public:
            ClassifierFindingEvent(int pFrameIndex, std::vector<cv::Point> pPoints);
        private:
            int findingFrameIndex;
            std::vector<cv::Point> points;
    };

    class Event
    {
        public:
            Event();

            // TODO: option 1.
            //          Event receives a EventDrawer and then calls draw on image for
            //          each frame that you pass with DrawOn(Frame, Drawer& ) : Frame,
            //       option 2.
            //          Allow to GetFinding(frameIndex) : Frame and then with
            //          polymorphism someone handles the draw call.


        private:
            std::vector<StandarFindingEvent> standarFindings;
            std::vector<ClassifierFindingEvent> classifierFindings;
    };

} // namespace Observer
