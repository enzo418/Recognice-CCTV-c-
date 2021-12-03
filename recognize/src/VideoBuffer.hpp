#pragma once

#include <optional>

#include "CircularFrameBuffer.hpp"
#include "RawCameraEvent.hpp"

namespace Observer {

    template <typename TFrame>
    class VideoBuffer {
       public:
        VideoBuffer(int sizeBufferBefore, int sizeBufferAfter);

        void ChangeWasDetected();

        bool AddFrame(TFrame& frame);

        bool CheckIfTheChangeIsValid();

        RawCameraEvent<TFrame> GetEventFound();

       private:
        bool changeDetected;
        int firstFrameWhereChangeWasFound;

        // delayed initialization with optional
        std::optional<CircularFrameBuffer<TFrame>> framesBefore;
        std::optional<CircularFrameBuffer<TFrame>> framesAfter;
    };



    template <typename TFrame>
    VideoBuffer<TFrame>::VideoBuffer(int sizeBufferBefore,
                                     int sizeBufferAfter) {
        this->framesBefore.emplace(sizeBufferBefore);
        this->framesAfter.emplace(sizeBufferAfter);

        this->changeDetected = false;
    }

    template <typename TFrame>
    void VideoBuffer<TFrame>::ChangeWasDetected() {
        this->changeDetected = true;
    }

    template <typename TFrame>
    bool VideoBuffer<TFrame>::AddFrame(TFrame& frame) {
        if (changeDetected) {
            return this->framesAfter->AddFrame(frame);
        } else {
            return this->framesBefore->AddFrame(frame);
        }
    }

    template <typename TFrame>
    bool VideoBuffer<TFrame>::CheckIfTheChangeIsValid() {
        // TODO
        return true;
    }

    template <typename TFrame>
    RawCameraEvent<TFrame> VideoBuffer<TFrame>::GetEventFound() {
        std::vector<TFrame> before = this->framesBefore->GetFrames();
        std::vector<TFrame> after = this->framesAfter->GetFrames();
        int firstFrameWhereChangeWasFound = before.size() - 1;

        std::vector<TFrame> merged(before.size() + after.size());
        std::swap_ranges(merged.begin(), merged.end() - before.size(),
                         before.begin());
        std::swap_ranges(merged.begin() + before.size(), merged.end(),
                         after.begin());

        RawCameraEvent ev(std::move(merged), firstFrameWhereChangeWasFound);

        return ev;
    }
}  // namespace Observer
