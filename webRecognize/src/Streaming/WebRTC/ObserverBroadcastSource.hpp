#pragma once

#include <atomic>
#include <webrtc/broadrtc/BroadcastSource.hpp>
//
#include <webrtc/rtc_base/time_utils.h>

#include <opencv2/opencv.hpp>
#include <thread>

#include "observer/Functionality.hpp"
#include "observer/IFrame.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Pattern/ObserverBasics.hpp"

namespace Web::Streaming::WebRTC {
    class ObserverSource final : public broadrtc::BroadcastSource,
                                 public ISubscriber<Observer::Frame> {
       public:
        ObserverSource() = default;

        virtual ~ObserverSource() {}

        bool Start() override;

        bool Stop() override;

        int GetWidth() override;
        int GetHeight() override;
        bool IsRunning() override;

        void OnNoAudience() override;

       private:
        void update(Observer::Frame frame) override;

       private:
        std::atomic_bool running {false};
        Observer::Size sourceSize;
    };
}  // namespace Web::Streaming::WebRTC