#pragma once

#include "BaseObserverPattern.hpp"
#include "Configuration.hpp"

namespace Observer {
    class FrameEventSubscriber : public ISubscriber<int, cv::Mat> {
        virtual void update(int camerapos, cv::Mat frame) = 0;
    };

    /**
    * @todo write docs
    */
    class FrameDisplay : public FrameEventSubscriber
    {
        public:
            void Start();

            void update(int camerapos, cv::Mat frame) override;

        private:
            std::vector<cv::Mat> frames;
    };
}
