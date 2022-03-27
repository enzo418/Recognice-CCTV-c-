#pragma once

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>
#include <utility>
#include <vector>

#include "../../Implementation.hpp"
#include "../../Log/log.hpp"
#include "../../Size.hpp"
#include "../../Utils/SpecialFunctions.hpp"
#include "../IVideoWriter.hpp"
#include "Notification.hpp"

namespace Observer {

    class VideoNotification : public Notification {
       public:
        VideoNotification(int groupID, Event ev, std::string text,
                          std::vector<Frame>&& frames);

        std::string GetCaption() override;

        std::vector<Frame>& GetFrames();

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

        void Resize(const Size& target);
        void Resize(double fx, double fy);

       private:
        std::string text;

        // absolute path
        std::string outputVideoPath;

        std::vector<Frame> frames;

        // video writer
        VideoWriter writer;

        int codec;

        double frameRate;

        Size frameSize;
    };
}  // namespace Observer
