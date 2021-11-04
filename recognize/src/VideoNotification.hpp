#pragma once

#include "Notification.hpp"
#include "utils/SpecialFunctions.hpp"
#include "OpencvVideoWriter.hpp"

#include <string>
#include <filesystem>
#include <vector>
#include <utility>

#include <opencv2/opencv.hpp>

namespace Observer
{

    namespace fs = std::filesystem;

    class VideoNotification : public Notification
    {
    public:
        VideoNotification(int groupID, Event ev, std::string text, std::vector<cv::Mat>&& frames);

        std::string GetCaption() override;

        std::string GetVideoPath();

        std::vector<cv::Mat>& GetFrames();

        /**
         * @brief Build a video notification.
         * Once built, you can get the path where it is saved with `GetVideoPath`
         * 
         * @param mediaFolderPath destination folder of the video
         * @param frameRate frame rate
         * @param codecID opencv codec id 
         * @param frameSize opencv frame size
         * @return true if could build the video
         * @return false 
         */
        bool BuildNotification(
            const std::string& mediaFolderPath, double frameRate, 
            int codecID, const cv::Size frameSize);

    private:
        std::string text;

        // absolute path
        std::string outputVideoPath;

        std::vector<cv::Mat> frames;            
        
        // video output
        OpencvVideoWritter writer;
    };

} // namespace Observer
