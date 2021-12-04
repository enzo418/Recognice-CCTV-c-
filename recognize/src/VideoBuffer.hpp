#pragma once

#include <optional>

#include "CircularFrameBuffer.hpp"
#include "RawCameraEvent.hpp"

namespace Observer {

    enum BufferState {
        BUFFER_IDLE = 0,  // idle -> nothing happend
        BUFFER_WAITING_FILL_UP_BEFORE = 1,
        BUFFER_WAITING_FILL_UP_AFTER = 2,  // wainting -> an event occurred
        BUFFER_READY = 3,  // before and after buffer are fullfilled
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

        bool CheckIfTheChangeIsValid();

        RawCameraEvent<TFrame> GetEventFound();

        int GetState();

       private:
        int firstFrameWhereChangeWasFound;

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
    bool VideoBuffer<TFrame>::CheckIfTheChangeIsValid() {
        // TODO
        return true;
    }

    template <typename TFrame>
    RawCameraEvent<TFrame> VideoBuffer<TFrame>::GetEventFound() {
        std::vector<TFrame> before = this->framesBefore.GetFrames();
        std::vector<TFrame> after = this->framesAfter.GetFrames();
        int firstFrameWhereChangeWasFound = before.size() - 1;

        std::vector<TFrame> merged(before.size() + after.size());
        std::swap_ranges(merged.begin(), merged.end() - before.size(),
                         before.begin());
        std::swap_ranges(merged.begin() + before.size(), merged.end(),
                         after.begin());

        RawCameraEvent ev(std::move(merged), firstFrameWhereChangeWasFound);

        return ev;
    }

    template <typename TFrame>
    int VideoBuffer<TFrame>::GetState() {
        return this->bufferState;
    }
}  // namespace Observer
