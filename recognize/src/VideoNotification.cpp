#include "VideoNotification.hpp"

namespace Observer
{
    VideoNotification::VideoNotification(
        int pGroupID, 
        Event pEvent, 
        std::string pText, 
        std::vector<cv::Mat>&& pFrames
    ) : text(pText), frames(std::move(pFrames)), Notification(pGroupID, pEvent) {}

    std::string VideoNotification::GetCaption() {
        return this->text;
    }

    std::string VideoNotification::GetVideoPath() {
        return this->outputVideoPath;
    }

    std::vector<cv::Mat>& VideoNotification::GetFrames() {
        return this->frames;
    }

    bool VideoNotification::BuildNotification(
        const std::string& mediaFolderPath, 
        double frameRate, int codecID,
        const cv::Size frameSize
        ) {
        const std::string time = Observer::SpecialFunctions::GetCurrentTime();
        const std::string fileName = time + ".mkv";
        const std::string &path = fs::path(mediaFolderPath) / fs::path(fileName);

        this->outputVideoPath = path;

        this->writer.Open(fileName, frameRate, codecID, frameSize);

        for (auto &&frame : this->frames)
        {
            this->writer.WriteFrame(frame);
        }
        
        this->writer.Close();
    }
} // namespace Observer
