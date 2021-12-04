#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

namespace Observer {

    template <typename TFrame>
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
        bool AddFrame(TFrame& frame);

        /**
         * @brief Returns the frames (not a copy), after
         * this call the instace BECOMES UNUSABLE, any
         * call to it's methods will cause a
         * segmentation fault.
         *
         * @return vector of frames
         */
        std::vector<TFrame> GetFrames();

       private:
        std::vector<TFrame> frames;

        int framesPosition;
    };


    template <typename TFrame>
    CircularFrameBuffer<TFrame>::CircularFrameBuffer(int bufferSize) {
        // reserve enough buffer size for the frames
        this->frames.resize(bufferSize);

        this->framesPosition = 0;
    }

    template <typename TFrame>
    bool CircularFrameBuffer<TFrame>::AddFrame(TFrame& frame) {
        this->frames[this->framesPosition] = frame.clone();

        const int next = this->framesPosition + 1;

        const bool isFull = next >= this->frames.capacity();
        this->framesPosition = isFull ? 0 : next;

        return isFull;
    }

    template <typename TFrame>
    std::vector<TFrame> CircularFrameBuffer<TFrame>::GetFrames() {
        // First order the frames

        // this->frames:
        // 0  1  2  3  4
        // --------------
        // N1 N2 N3 N4 N5 -> pos = 0; for i = 0 < capacity
        // N5 N1 N2 N3 N4 -> pos = 1; for i = 1 < capacity & for i = 0 < pos
        // N4 N5 N1 N2 N3 -> pos = 2; for i = 2 < capacity & for i = 0 < pos
        // N3 N4 N5 N1 N2 -> pos = 3; for i = 3 < capacity & for i = 0 < pos
        // N2 N3 N4 N5 N1 -> pos = 4; for i = 4 < capacity & for i = 0 < pos

        if (this->framesPosition != 0) {
            // initialize
            std::vector<TFrame> ordered(this->frames.capacity());

            // Swap may not be so easy to read as a for loop but
            // it can save up to 30% on memory and 20% on heap allocations
            // according to the tests I've done.

            // Assume framesPosition = 2

            // swap from "frames" starting from N1 to N3, leaving it "ordered"
            // starting from position 0 -> N1 N2 N3
            std::swap_ranges(ordered.begin(),

                             ordered.end() - this->framesPosition,

                             frames.begin() + this->framesPosition);

            // swap from "frames" starting from N4 to N5, leaving it "ordered"
            // starting from position 3
            std::swap_ranges(ordered.begin() + (this->frames.capacity() -
                                                this->framesPosition),

                             ordered.end(),

                             frames.begin());

            return ordered;
        }

        return std::move(this->frames);
    }
}  // namespace Observer
