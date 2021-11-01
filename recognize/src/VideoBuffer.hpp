#pragma once

#include "CircularFrameBuffer.hpp"
#include "RawCameraEvent.hpp"

#include <optional>

namespace Observer
{
    class VideoBuffer
    {
        public:
            VideoBuffer(int sizeBufferBefore, int sizeBufferAfter);

            void ChangeWasDetected();

            bool AddFrame(cv::Mat &frame);

            bool CheckIfTheChangeIsValid();

            RawCameraEvent GetEventFound();

        private:
            bool changeDetected;
            int firstFrameWhereChangeWasFound;

            // delayed initialization with optional
            std::optional<CircularFrameBuffer> framesBefore;
            std::optional<CircularFrameBuffer> framesAfter;
    };
} // namespace Observer
