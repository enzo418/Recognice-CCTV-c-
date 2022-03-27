#pragma once

#include "../../src/Domain/IVideoWriter.hpp"
#include "Frame.hpp"

namespace Observer {
    class VideoWriter : public IVideoWriter<Frame> {
       private:
        cv::VideoWriter writer;

       public:
        VideoWriter() = default;

        bool Open(const std::string& path, const double& framerate,
                  const int& codecID, const Size& frameSize) override;

        void Close() override;

        void WriteFrame(Frame& frame) override;

        int GetDefaultCodec() override;
    };
}  // namespace Observer
