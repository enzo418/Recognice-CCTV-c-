#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

namespace Observer {
    /**
     * @brief Holds all the frames and the
     * first frame where a change was found.
     */
    template <typename TFrame>
    struct CameraEvent {
       public:
        CameraEvent() = default;

        // && -> pFrames can only be r-value (std::move is necessary)
        CameraEvent(std::vector<TFrame>&& pFrames, int pIndexFirst)
            : frames(std::move(pFrames)),
              frameIndexOfFirstChange(pIndexFirst) {}

        TFrame& GetFrameAt(int index) &;

        std::vector<TFrame>& GetFrames() &;

        /**
         * Returns a move instructor of the frames,
         * this object should be deleted after this call.
         * @return rvalue - frames
         */
        std::vector<TFrame> PopFrames();

       private:
        std::vector<TFrame> frames;
        int frameIndexOfFirstChange {};
    };

    template <typename TFrame>
    TFrame& CameraEvent<TFrame>::GetFrameAt(int index) & {
        return this->frames[index];
    }

    template <typename TFrame>
    std::vector<TFrame>& CameraEvent<TFrame>::GetFrames() & {
        return this->frames;
    }

    template <typename TFrame>
    std::vector<TFrame> CameraEvent<TFrame>::PopFrames() {
        return std::move(this->frames);
    }
}  // namespace Observer