#pragma once

#include <opencv2/opencv.hpp>
#include <string>

#include "../Size.hpp"

namespace Observer {  // all the implementations (VideoWriter) will provide a
                      // type
    template <typename T>
    class VideoWriter;

    template <typename TFrame>
    class IVideoWriter {
       public:
        IVideoWriter() = default;

        virtual bool Open(const std::string& string, const double& framerate,
                          const int& codecID, const Size& frameSize) = 0;

        virtual void Close() = 0;

        virtual void WriteFrame(TFrame& frame) = 0;

        virtual int GetDefaultCodec() = 0;
    };

}  // namespace Observer
