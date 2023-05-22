#include "VideoSource.hpp"

#include <opencv2/videoio.hpp>

namespace Observer {
    void VideoSource::Open(const std::string& url) {
        try {
            if (std::all_of(url.begin(), url.end(), ::isdigit)) {
                this->videoCapture.open(std::stoi(url));
            } else {
                this->videoCapture.open(url);
            }
        } catch (...) {
            // ffmpeg if cannot open it terminates the program, so we need
            // to catch it. To know if the source is ok check for isOpened
        }
    }

    void VideoSource::Close() { this->videoCapture.release(); }

    bool VideoSource::GetNextFrame(Frame& frame) {
        return this->videoCapture.read(frame.GetInternalFrame());
    }

    bool VideoSource::isOpened() { return this->videoCapture.isOpened(); }

    int VideoSource::GetFPS() { return videoCapture.get(cv::CAP_PROP_FPS); }

    Size VideoSource::GetSize() {
        return Size {(int)videoCapture.get(cv::CAP_PROP_FRAME_WIDTH),
                     (int)videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT)};
    }

}  // namespace Observer
