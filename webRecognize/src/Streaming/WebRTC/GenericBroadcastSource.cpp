#include "GenericBroadcastSource.hpp"

namespace Web::Streaming::WebRTC {
    bool GenericSource::Start() {
        broadrtc::BroadcastSource::Start();

        OBSERVER_ASSERT(!m_running, "GenericSource already running");

        m_running = true;
        m_thread = std::thread(&GenericSource::CaptureLoop, this);

        return true;
    }

    bool GenericSource::Stop() {
        broadrtc::BroadcastSource::Stop();

        OBSERVER_ASSERT(m_running, "GenericSource not running");

        m_running = false;
        if (m_thread.joinable()) {
            m_thread.join();
        }

        return true;
    }

    int GenericSource::GetWidth() { return sourceSize.width; }
    int GenericSource::GetHeight() { return sourceSize.height; }
    bool GenericSource::IsRunning() { return m_running; }

    void GenericSource::OnNoAudience() {
        std::cout << "GenericSource has no audience." << std::endl;
        this->Stop();
    }

    void GenericSource::CaptureLoop() {
        m_capture.Open(uri);
        if (!m_capture.isOpened()) {
            OBSERVER_WARN("GenericSource::CaptureLoop failed to open uri: {}",
                          uri);
            return;
        }

        sourceSize = m_capture.GetSize();

        Observer::Frame frame;
        while (m_running) {
            // get frame
            if (!m_capture.GetNextFrame(frame)) {
                OBSERVER_WARN(
                    "GenericSource::CaptureLoop failed to read frame");
                break;
            }

            frame.ToColorSpace(Observer::ColorSpaceConversion::COLOR_BGR2RGB);

            int width = sourceSize.width;
            int height = sourceSize.height;
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
    }

}  // namespace Web::Streaming::WebRTC