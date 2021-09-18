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

        int pos = next > this->frames.capacity() ? 0 : next;
    }

    std::vector<cv::Mat> CicularFrameBuffer::GetFrames() {
        // TODO
    }
}