#include "VideoNotification.hpp"

namespace Observer {

    namespace fs = std::filesystem;

    VideoNotification::VideoNotification(int pGroupID, Event pEvent,
                                         std::vector<Frame>&& pFrames,
                                         std::string pOutputFolder)
        : Notification(pGroupID, std::move(pEvent),
                       VideoNotification::CreatePath(pOutputFolder)),
          frames(std::move(pFrames)),
          codec(-418) {}

    std::vector<Frame>& VideoNotification::GetFrames() { return this->frames; }

    void VideoNotification::BuildNotification() {
        // GetContent in this context is the path to the video
        this->writer.Open(
            this->GetContent(), this->frameRate,
            this->codec == -418 ? this->writer.GetDefaultCodec() : this->codec,
            this->frameSize);

        for (auto& frame : this->frames) {
            this->writer.WriteFrame(frame);
        }

        this->writer.Close();
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

    std::string VideoNotification::CreatePath(
        const std::string& pOutputFolder) {
        const std::string time = Observer::SpecialFunctions::GetCurrentTime();
        const std::string fileName = time + ".mp4";
        const std::string& path = fs::path(pOutputFolder) / fs::path(fileName);
        return path;
    }
}  // namespace Observer
