#pragma once

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>
#include <utility>
#include <vector>

#include "../../ImageTransformation.hpp"
#include "../../Log/log.hpp"
#include "../../Size.hpp"
#include "../../Utils/SpecialFunctions.hpp"
#include "../VideoWriter.hpp"
#include "Notification.hpp"

namespace Observer {

    namespace fs = std::filesystem;

    template <typename TFrame>
    class VideoNotification : public Notification {
       public:
        VideoNotification(int groupID, Event ev, std::string text,
                          std::vector<TFrame>&& frames);

        std::string GetCaption() override;

        std::vector<TFrame>& GetFrames();

        /**
         * @brief Build a video notification and return the path.
         *
         * @param mediaFolderPath destination folder of the video
         * @param frameRate frame rate
         * @param codecID opencv codec id
         * @param frameSize opencv frame size
         * @return video path
         */
        std::string BuildNotification(const std::string& mediaFolderPath);

        void SetCodec(int codec);
        void SetFrameRate(double frameRate);
        void SetFrameSize(Size frameSize);

       private:
        std::string text;

        // absolute path
        std::string outputVideoPath;

        std::vector<TFrame> frames;

        // video writer
        VideoWriter<TFrame> writer;

        int codec;

        double frameRate;

        Size frameSize;
    };

    template <typename TFrame>
    VideoNotification<TFrame>::VideoNotification(int pGroupID, Event pEvent,
                                                 std::string pText,
                                                 std::vector<TFrame>&& pFrames)
        : text(std::move(pText)),
          frames(std::move(pFrames)),
          Notification(pGroupID, std::move(pEvent)),
          codec(-418) {}

    template <typename TFrame>
    std::string VideoNotification<TFrame>::GetCaption() {
        return this->text;
    }

    template <typename TFrame>
    std::vector<TFrame>& VideoNotification<TFrame>::GetFrames() {
        return this->frames;
    }

    template <typename TFrame>
    std::string VideoNotification<TFrame>::BuildNotification(
        const std::string& mediaFolderPath) {
        OBSERVER_ASSERT(!frames.empty(),
                        "Empty frames while building a video notification");

        const std::string time = Observer::SpecialFunctions::GetCurrentTime();
        const std::string fileName = time + ".mp4";
        const std::string& path =
            fs::path(mediaFolderPath) / fs::path(fileName);

        this->writer.Open(
            path, this->frameRate,
            this->codec == -418 ? this->writer.GetDefaultCodec() : this->codec,
            this->frameSize);

        for (auto&& frame : this->frames) {
            this->writer.WriteFrame(frame);
        }

        this->writer.Close();

        return path;
    }

    template <typename TFrame>
    void VideoNotification<TFrame>::SetCodec(int pCodec) {
        this->codec = pCodec;
    }

    template <typename TFrame>
    void VideoNotification<TFrame>::SetFrameRate(double pFrameRate) {
        this->frameRate = pFrameRate;
    }

    template <typename TFrame>
    void VideoNotification<TFrame>::SetFrameSize(Size pFrameSize) {
        this->frameSize = pFrameSize;
    }
}  // namespace Observer
