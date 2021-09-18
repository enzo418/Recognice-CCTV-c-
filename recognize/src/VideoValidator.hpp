#pragma once

#include "CicularFrameBuffer.hpp"
#include <optional>

namespace Observer
{
    class VideoValidator
    {
        public:
            VideoValidator(int sizeBufferBefore, int sizeBufferAfter);

            void ChangeWasDetected();

            bool AddFrame(cv::Mat &frame);

            bool CheckIfTheChangeIsValid();

            std::vector<cv::Mat> GetFrames();

        private:
            bool changeDetected;

            // delayed initialization with optional
            std::optional<CicularFrameBuffer> framesBefore;
            std::optional<CicularFrameBuffer> framesAfter;
    };
} // namespace Observer
