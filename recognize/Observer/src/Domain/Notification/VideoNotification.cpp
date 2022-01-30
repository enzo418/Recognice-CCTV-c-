#include "VideoNotification.hpp"

namespace Observer {

    namespace fs = std::filesystem;

    VideoNotification::VideoNotification(int pGroupID, Event pEvent,
                                         std::string pText,
                                         std::vector<Frame>&& pFrames)
        : text(std::move(pText)),
          frames(std::move(pFrames)),
          Notification(pGroupID, std::move(pEvent)),
          codec(-418) {}

    std::string VideoNotification::GetCaption() { return this->text; }

    std::vector<Frame>& VideoNotification::GetFrames() { return this->frames; }

    std::string VideoNotification::BuildNotification(
        const std::string& mediaFolderPath) {
        const std::string time = Observer::SpecialFunctions::GetCurrentTime();
        const std::string fileName = time + ".mp4";
        const std::string& path =
            fs::path(mediaFolderPath) / fs::path(fileName);

        this->writer.Open(
            path, this->frameRate,
            this->codec == -418 ? this->writer.GetDefaultCodec() : this->codec,
            this->frameSize);

        for (auto& frame : this->frames) {
            this->writer.WriteFrame(frame);
        }

        this->writer.Close();

        return path;
    }

    void VideoNotification::SetCodec(int pCodec) { this->codec = pCodec; }

    void VideoNotification::SetFrameRate(double pFrameRate) {
        this->frameRate = pFrameRate;
    }

    void VideoNotification::SetFrameSize(Size pFrameSize) {
        this->frameSize = pFrameSize;
    }

    void VideoNotification::Resize(const Size& target) {
        this->SetFrameSize(target);

        for (auto& frame : frames) {
            frame.Resize(target);
        }
    }

    void VideoNotification::Resize(double fx, double fy) {
        Size size = frames[0].GetSize();
        size.width *= fx;
        size.height *= fy;

        this->Resize(size);
    }
}  // namespace Observer
