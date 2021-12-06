#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

#include "../../Size.hpp"

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

        double GetFrameRate();

        void SetFrameRate(double pFrameRate);

        /**
         * Returns a move instructor of the frames,
         * this object should be deleted after this call.
         * @return rvalue - frames
         */
        std::vector<TFrame> PopFrames();

       private:
        std::vector<TFrame> frames;
        int frameIndexOfFirstChange {};

        // frame rate of the recorded video
        double frameRate;

        Size framesSize;
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

    template <typename TFrame>
    double CameraEvent<TFrame>::GetFrameRate() {
        return this->frameRate;
    }

    template <typename TFrame>
    void CameraEvent<TFrame>::SetFrameRate(double pFrameRate) {
        this->frameRate = pFrameRate;
    }

}  // namespace Observer