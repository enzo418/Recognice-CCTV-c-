#pragma once

#include <optional>

#include "../CircularFrameBuffer.hpp"
#include "../Implementation.hpp"
#include "../Log/log.hpp"
#include "Event/CameraEvent.hpp"

namespace Observer {

    enum BufferState {
        BUFFER_IDLE = 0,  // idle -> nothing happend
        BUFFER_WAITING_FILL_UP_BEFORE = 1,
        BUFFER_WAITING_FILL_UP_AFTER = 2,  // wainting -> an event occurred
        BUFFER_READY = 3,     // before and after buffer are fullfilled
        BUFFER_UNDEFINED = 4  // buffer is almost in an undefined bhvr.
    };

    class VideoBuffer {
       public:
        VideoBuffer(int sizeBufferBefore, int sizeBufferAfter);

        void ChangeWasDetected();

        /**
         * @brief Adds a frame to the buffer.
         * Returns the state of the buffer after adding the frame.
         * @param frame frame to add
         * @return int state of the buffer (BufferState)
         */
        int AddFrame(Frame& frame);

        /**
         * @brief Returns all frames in the buffer in a single list,
         * sorted with respect to the time they were added.
         * After this call no call to AddFrames should be done.
         *
         * @return std::vector<Frame>
         */
        std::vector<Frame> PopAllFrames();

        /**
         * @brief Get the index of the middle frame of the buffer.
         *
         * @return int
         */
        int GetIndexMiddleFrame();

        int GetState();

       private:
        int indexMiddleFrame;

        // delayed initialization with optional
        CircularFrameBuffer framesBefore;
        CircularFrameBuffer framesAfter;

        int bufferState;
    };
}  // namespace Observer
