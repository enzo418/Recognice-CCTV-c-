#pragma once

#include <opencv2/opencv.hpp>
#include <string>

#include "observer/Size.hpp"

namespace Observer {
    // all the implementations (VideoSource) will provide a type
    class VideoSource;

    template <typename TFrame>
    class IVideoSource {
       public:
        virtual void Open(const std::string& url) = 0;

        virtual void Close() = 0;

        virtual bool GetNextFrame(TFrame&) = 0;

        virtual bool isOpened() = 0;

        virtual int GetFPS() = 0;

        virtual Size GetSize() = 0;
    };
}  // namespace Observer
