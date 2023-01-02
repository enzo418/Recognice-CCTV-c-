#pragma once

#include "Frame.hpp"
#include "observer/Domain/IVideoSource.hpp"

namespace Observer {
    class VideoSource final : public IVideoSource<Frame> {
       public:
        VideoSource() = default;

        void Open(const std::string& url) override;

        void Close() override;

        bool GetNextFrame(Frame& frame) override;

        bool isOpened() override;

        int GetFPS() override;

        Size GetSize() override;

       private:
        cv::VideoCapture videoCapture;
    };
}  // namespace Observer
