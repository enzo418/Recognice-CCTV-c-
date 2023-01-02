#pragma once

#include <vector>

#include "Implementation.hpp"

namespace Observer {

    class CircularFrameBuffer {
       public:
        /**
         * @brief Construct a new Cicular Frame Buffer object
         *
         * @param bufferSize max buffer size
         */
        CircularFrameBuffer(int bufferSize);

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
        bool AddFrame(Frame& frame);

        /**
         * @brief Returns the frames (not a copy), after
         * this call the instace BECOMES UNUSABLE, any
         * call to it's methods will cause a
         * segmentation fault.
         *
         * @return vector of frames
         */
        std::vector<Frame> GetFrames();

        bool IsFull();

       private:
        std::vector<Frame> frames;

        int bufferSize;

        int framesPosition;

        int elementCount {0};
    };
}  // namespace Observer
