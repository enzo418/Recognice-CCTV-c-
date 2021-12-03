#pragma once

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>
#include <utility>
#include <vector>

#include "Notification.hpp"
#include "Size.hpp"
#include "VideoWriter.hpp"
#include "utils/SpecialFunctions.hpp"

namespace Observer {

    namespace fs = std::filesystem;

    template <typename TFrame>
    class VideoNotification : public Notification {
       public:
        VideoNotification(int groupID, Event ev, std::string text,
                          std::vector<TFrame>&& frames);

        std::string GetCaption() override;

        std::string GetVideoPath();

        std::vector<TFrame>& GetFrames();

        /**
         * @brief Build a video notification.
         * Once built, you can get the path where it is saved with
         * `GetVideoPath`
         *
         * @param mediaFolderPath destination folder of the video
         * @param frameRate frame rate
         * @param codecID opencv codec id
         * @param frameSize opencv frame size
         * @return true if could build the video
         * @return false
         */
        bool BuildNotification(const std::string& mediaFolderPath,
                               double frameRate, int codecID,
                               const Size frameSize);

       private:
        std::string text;

        // absolute path
        std::string outputVideoPath;

        std::vector<TFrame> frames;

        // video writer
        VideoWriter<TFrame> writer;
    };

    template <typename TFrame>
    VideoNotification<TFrame>::VideoNotification(int pGroupID, Event pEvent,
                                                 std::string pText,
                                                 std::vector<TFrame>&& pFrames)
        : text(std::move(pText)),
          frames(std::move(pFrames)),
          Notification(pGroupID, std::move(pEvent)) {}

    template <typename TFrame>
    std::string VideoNotification<TFrame>::GetCaption() {
        return this->text;
    }

    template <typename TFrame>
    std::string VideoNotification<TFrame>::GetVideoPath() {
        return this->outputVideoPath;
    }

    template <typename TFrame>
    std::vector<TFrame>& VideoNotification<TFrame>::GetFrames() {
        return this->frames;
    }

    template <typename TFrame>
    bool VideoNotification<TFrame>::BuildNotification(
        const std::string& mediaFolderPath, double frameRate, int codecID,
        const Size frameSize) {
        const std::string time = Observer::SpecialFunctions::GetCurrentTime();
        const std::string fileName = time + ".mkv";
        const std::string& path =
            fs::path(mediaFolderPath) / fs::path(fileName);

        this->outputVideoPath = path;

        this->writer.Open(fileName, frameRate, codecID, frameSize);

        for (auto&& frame : this->frames) {
            this->writer.WriteFrame(frame);
        }

        this->writer.Close();

        return true;
    }
}  // namespace Observer
