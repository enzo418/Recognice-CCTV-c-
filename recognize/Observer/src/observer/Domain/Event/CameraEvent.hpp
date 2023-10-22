#pragma once

#include <vector>

#include "observer/Implementation.hpp"
#include "observer/Size.hpp"

namespace Observer {
    /**
     * @brief Holds all the event frames and the first frame where a change was
     * found.
     */
    struct CameraEvent {
       public:
        CameraEvent() = default;

        // && -> pFrames can only be r-value (std::move is necessary)
        CameraEvent(std::vector<Frame>&& pFrames, int pIndexFirst)
            : frames(std::move(pFrames)),
              frameIndexOfFirstChange(pIndexFirst) {}

        Frame& GetFrameAt(int index) &;

        std::vector<Frame>& GetFrames() &;

        double GetFrameRate() const;
        Size GetFramesSize() const;
        int GetGroupID() const;

        void SetFrameRate(double pFrameRate);
        void SetFrameSize(Size pFrameSize);

        /**
         * @brief Sets the group id.
         * Once it's a valid event we set the group id.
         *
         * @param groupID
         */
        void SetGroupID(int groupID);

        /**
         * Returns a move instructor of the frames,
         * this object should be deleted after this call.
         * @return rvalue - frames
         */
        std::vector<Frame> PopFrames();

       private:
        std::vector<Frame> frames;
        int frameIndexOfFirstChange {};

        // frame rate of the recorded video
        double frameRate {0};

        Size framesSize;

        // unique value in the program execution
        int groupID {-1};
    };
}  // namespace Observer