#pragma once

#include <atomic>
#include <webrtc/broadrtc/BroadcastSource.hpp>
//
#include <webrtc/rtc_base/time_utils.h>

#include <opencv2/opencv.hpp>
#include <thread>

#include "observer/Functionality.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"

namespace Web::Streaming::WebRTC {

    class GenericSource final : public broadrtc::BroadcastSource {
       public:
        GenericSource(const std::string& pUri) : uri(pUri) {}

        virtual ~GenericSource() {}

        bool Start() override;

        bool Stop() override;

        int GetWidth() override;
        int GetHeight() override;
        bool IsRunning() override;

        void OnNoAudience() override;

       private:
        void CaptureLoop();

       private:
        std::thread m_thread;
        std::atomic_bool m_running {false};

        // capture
        Observer::VideoSource m_capture;
        std::string uri;
        Observer::Size sourceSize;
    };
}  // namespace Web::Streaming::WebRTC