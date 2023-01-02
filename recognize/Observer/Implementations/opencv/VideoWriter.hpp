#pragma once

#include "Frame.hpp"
#include "observer/Domain/IVideoWriter.hpp"

namespace Observer {
    class VideoWriter final : public IVideoWriter<Frame> {
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
