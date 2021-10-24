#include "CicularFrameBuffer.hpp"

namespace Observer {
    CicularFrameBuffer::CicularFrameBuffer(int bufferSize) {
        // reserve enough buffer size for the frames
        this->frames.reserve(bufferSize);

        this->framesPosition = 0;
    }

    bool CicularFrameBuffer::AddFrame(cv::Mat& frame) {
        this->frames[this->framesPosition] = frame.clone();
        
        const int next = this->framesPosition + 1;

        const bool isFull = next > this->frames.capacity();
        this->framesPosition = isFull ? 0 : next;

        return isFull;
    }

    std::vector<cv::Mat> CicularFrameBuffer::GetFrames() {
        // First order the frames

        // 0  1  2  3  4
        // --------------
        // N1 N2 N3 N4 N5 -> pos = 0; for i = 0 < capacity
        // N5 N1 N2 N3 N4 -> pos = 1; for i = 1 < capacity & for i = 0 < pos
        // N4 N5 N1 N2 N3 -> pos = 2; for i = 2 < capacity & for i = 0 < pos
        // N3 N4 N5 N1 N2 -> pos = 3; for i = 3 < capacity & for i = 0 < pos
        // N2 N3 N4 N5 N1 -> pos = 4; for i = 4 < capacity & for i = 0 < pos

        if (this->framesPosition != 0) {
            std::vector<cv::Mat> ordered;

            for(int i = this->framesPosition; i < this->frames.capacity(); i++) {
                ordered.push_back(std::move(this->frames[i]));
            }

            for(int i = 0; i < this->framesPosition; i++) {
                ordered.push_back(std::move(this->frames[i]));
            }

            return ordered;
        }

        return std::move(this->frames);
    }
}
