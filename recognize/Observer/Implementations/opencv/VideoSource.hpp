#pragma once

#include "../../src/Domain/IVideoSource.hpp"
#include "Frame.hpp"

namespace Observer {
    class VideoSource : public IVideoSource<Frame> {
       public:
        VideoSource() = default;

        void Open(const std::string& url) override;

        void Close() override;

        bool GetNextFrame(Frame& frame) override;

        bool isOpened() override;

        int GetFPS() override;

       private:
        cv::VideoCapture videoCapture;
    };
}  // namespace Observer
