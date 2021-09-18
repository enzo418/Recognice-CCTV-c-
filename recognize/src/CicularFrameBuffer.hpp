#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

namespace Observer
{
    class CicularFrameBuffer {
        public:
            /**
             * @brief Construct a new Cicular Frame Buffer object
             * 
             * @param bufferSize max buffer size
             */
            CicularFrameBuffer(int bufferSize);
            
            /**
             * @brief Adds a frame to the buffer.
             * After the buffer is full it starts replacing
             * the oldest frame with the new ones.
             * 
             * @param frame 
             * @return true if the buffer is full
             * @return false if the frame is not full
             * 
             */
            bool AddFrame(cv::Mat& frame);

            /**
             * @brief Get the frames
             * 
             * @return vector of frames
             */
            std::vector<cv::Mat> GetFrames();

        private:
            std::vector<cv::Mat> frames;

            int framesPosition;
    };
} // namespace Observer
