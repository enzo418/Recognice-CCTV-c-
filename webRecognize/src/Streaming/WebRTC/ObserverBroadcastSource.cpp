#include "ObserverBroadcastSource.hpp"

namespace Web::Streaming::WebRTC {
    bool ObserverSource::Start() {
        broadrtc::BroadcastSource::Start();

        OBSERVER_ASSERT(!running, "ObserverSource already running");

        running = true;

        return true;
    }

    bool ObserverSource::Stop() {
        broadrtc::BroadcastSource::Stop();

        OBSERVER_ASSERT(running, "ObserverSource not running");

        running = false;

        return true;
    }

    int ObserverSource::GetWidth() { return sourceSize.width; }
    int ObserverSource::GetHeight() { return sourceSize.height; }
    bool ObserverSource::IsRunning() { return running; }

    void ObserverSource::OnNoAudience() {
        OBSERVER_TRACE("ObserverBroadcastSource source has no audience.");
        this->Stop();
    }

    void ObserverSource::update(Observer::Frame frame) {
        if (!running || frame.IsEmpty()) return;

        if (frame.GetSize() != sourceSize) {
            sourceSize = frame.GetSize();
        }

        frame.ToColorSpace(Observer::ColorSpaceConversion::COLOR_BGR2RGB);

        const int width = sourceSize.width;
        const int height = sourceSize.height;

        rtc::scoped_refptr<webrtc::I420Buffer> I420buffer =
            webrtc::I420Buffer::Create(width, height);

        int res = libyuv::ConvertToI420(
            frame.GetData(), width * 3, I420buffer->MutableDataY(),
            I420buffer->StrideY(), I420buffer->MutableDataU(),
            I420buffer->StrideU(), I420buffer->MutableDataV(),
            I420buffer->StrideV(), 0, 0, width, height, width, height,
            libyuv::kRotate0, libyuv::FOURCC_RGB3);

        if (res >= 0) {
            webrtc::VideoFrame videoFrame =
                webrtc::VideoFrame::Builder()
                    .set_video_frame_buffer(I420buffer)
                    .set_rotation(webrtc::kVideoRotation_0)
                    .set_timestamp_us(rtc::TimeMicros())
                    .build();

            this->OnFrame(videoFrame);
        } else {
            OBSERVER_WARN("Conversion error: {}", res);
        }
    }
}  // namespace Web::Streaming::WebRTC