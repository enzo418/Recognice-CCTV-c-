#pragma once

#include "CameraObserver.hpp"

namespace Observer {
    class PasiveCamera : public CameraObserver {
       public:
        explicit PasiveCamera(CameraConfiguration* configuration);

       public:
        void SubscribeToCameraEvents(
            ICameraEventSubscriber* subscriber) override;

        void SubscribeToThresholdUpdate(
            IThresholdEventSubscriber* subscriber) override;

       protected:
        void ProcessFrame(Frame& frame) override;
    };
}  // namespace Observer