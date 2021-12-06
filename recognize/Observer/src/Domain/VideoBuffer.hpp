#pragma once

#include <optional>

#include "../CircularFrameBuffer.hpp"
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

    template <typename TFrame>
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
        int AddFrame(TFrame& frame);

        /**
         * @brief Returns all frames in the buffer in a single list,
         * sorted with respect to the time they were added.
         * After this call no call to AddFrames should be done.
         *
         * @return std::vector<TFrame>
         */
        std::vector<TFrame> PopAllFrames();

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
        CircularFrameBuffer<TFrame> framesBefore;
        CircularFrameBuffer<TFrame> framesAfter;

        int bufferState;
    };

    template <typename TFrame>
    VideoBuffer<TFrame>::VideoBuffer(int sizeBufferBefore, int sizeBufferAfter)
        : framesBefore(sizeBufferBefore), framesAfter(sizeBufferAfter) {
        this->bufferState = BUFFER_IDLE;
    }

    template <typename TFrame>
    void VideoBuffer<TFrame>::ChangeWasDetected() {
        this->bufferState = BUFFER_WAITING_FILL_UP_BEFORE;
    }

    template <typename TFrame>
    int VideoBuffer<TFrame>::AddFrame(TFrame& frame) {
        OBSERVER_ASSERT(
            this->bufferState != BUFFER_UNDEFINED,
            "No call to AddFrame should be done after PopAllFrames");

        if (this->bufferState == BUFFER_WAITING_FILL_UP_AFTER) {
            if (this->framesAfter.AddFrame(frame)) {
                this->bufferState = BUFFER_READY;
            }
        } else {
            // if it was wainting to fill the before buffer,
            // and AddFrame returned true (sub-buffer is full)
            if (this->framesBefore.AddFrame(frame) &&
                this->bufferState == BUFFER_WAITING_FILL_UP_BEFORE) {
                this->bufferState = BUFFER_WAITING_FILL_UP_AFTER;
            }
        }

        return bufferState;
    }

    template <typename TFrame>
    std::vector<TFrame> VideoBuffer<TFrame>::PopAllFrames() {
        this->bufferState = BUFFER_UNDEFINED;

        std::vector<TFrame> before = this->framesBefore.GetFrames();
        std::vector<TFrame> after = this->framesAfter.GetFrames();
        this->indexMiddleFrame = before.size() - 1;

        std::vector<TFrame> merged(before.size() + after.size());
        std::swap_ranges(merged.begin(), merged.end() - before.size(),
                         before.begin());
        std::swap_ranges(merged.begin() + before.size(), merged.end(),
                         after.begin());

        return std::move(merged);
    }

    template <typename TFrame>
    int VideoBuffer<TFrame>::GetState() {
        return this->bufferState;
    }

    template <typename TFrame>
    int VideoBuffer<TFrame>::GetIndexMiddleFrame() {
        return this->indexMiddleFrame;
    }
}  // namespace Observer
