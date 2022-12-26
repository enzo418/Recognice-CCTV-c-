#include "VideoBuffer.hpp"

namespace Observer {

    VideoBuffer::VideoBuffer(int sizeBufferBefore, int sizeBufferAfter)
        : framesBefore(sizeBufferBefore), framesAfter(sizeBufferAfter) {
        this->bufferState = BUFFER_IDLE;
    }

    void VideoBuffer::ChangeWasDetected() {
        if (this->framesBefore.IsFull()) {
            this->bufferState = BUFFER_WAITING_FILL_UP_AFTER;
        } else {
            this->bufferState = BUFFER_WAITING_FILL_UP_BEFORE;
        }
    }

    int VideoBuffer::AddFrame(Frame& frame) {
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

    std::vector<Frame> VideoBuffer::PopAllFrames() {
        this->bufferState = BUFFER_UNDEFINED;

        std::vector<Frame> before = this->framesBefore.GetFrames();
        std::vector<Frame> after = this->framesAfter.GetFrames();
        this->indexMiddleFrame = before.size() - 1;

        std::vector<Frame> merged(before.size() + after.size());
        std::swap_ranges(merged.begin(), merged.end() - before.size(),
                         before.begin());
        std::swap_ranges(merged.begin() + before.size(), merged.end(),
                         after.begin());

        return merged;
    }

    int VideoBuffer::GetState() { return this->bufferState; }

    int VideoBuffer::GetIndexMiddleFrame() { return this->indexMiddleFrame; }
}  // namespace Observer
