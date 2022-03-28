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

    /**
     * @brief A notification that is able to create a video
     * from frames
     */
    class VideoNotification : public Notification {
       public:
        VideoNotification(int groupID, Event ev, std::vector<Frame>&& frames,
                          std::string outputFolder);

        std::vector<Frame>& GetFrames();

        /**
         * @brief Build a video notification and return the path.
         *
         */
        void BuildNotification();

        void SetCodec(int codec);
        void SetFrameRate(double frameRate);
        void SetFrameSize(Size frameSize);

        void Resize(const Size& target);
        void Resize(double fx, double fy);

       private:
        std::vector<Frame> frames;

        // video writer
        VideoWriter writer;

        int codec;

        double frameRate;

        Size frameSize;

       protected:
        /**
         * @brief Creates a unique path for a .mp4 video file
         *
         * @param outputFolder
         * @return std::string
         */
        static std::string CreatePath(const std::string& outputFolder);
    };
}  // namespace Observer
