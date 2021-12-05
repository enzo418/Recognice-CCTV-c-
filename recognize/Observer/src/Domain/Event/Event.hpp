#pragma once

#include <opencv2/opencv.hpp>
#include <utility>

#include "../../Point.hpp"
#include "../../Rect.hpp"

namespace Observer {
    class StandarFindingEvent {
       public:
        StandarFindingEvent(int pFrameIndex, Point pCenter, Rect pRect);

        int GetFindingIndex();

        Point GetCenter();

        Rect GetRect();

       private:
        int findingFrameIndex;
        Point center;
        Rect rect;
    };

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

        void AddStandarFinding(StandarFindingEvent&& finding);
        void AddClassifierFinding(ClassifierFindingEvent&& finding);

        void SetCameraName(std::string cameraName);

        std::string GetCameraName();

        int GetFirstFrameWhereFindingWasFound();

        // TODO: Who draws the findings?
        //      option 1.
        //          Event receives a EventDrawer and then calls draw on image
        //          for each frame that you pass with DrawOn(Frame, Drawer& ) :
        //          Frame,
        //       option 2.
        //          Allow to GetFinding(frameIndex) : Frame and then with
        //          polymorphism someone handles the draw call.

       private:
        // camera
        std::string cameraName;

        // frames/findings
        std::vector<StandarFindingEvent> standarFindings;
        std::vector<ClassifierFindingEvent> classifierFindings;
    };

}  // namespace Observer
