#pragma once

#include "CameraObserver.hpp"

namespace Observer {
    class PassiveCamera final : public CameraObserver {
       public:
        explicit PassiveCamera(CameraConfiguration* configuration);

       public:
        void SubscribeToCameraEvents(
            ICameraEventSubscriber* subscriber) override;

        void SubscribeToThresholdUpdate(
            IThresholdEventSubscriber* subscriber) override;

       protected:
        void ProcessFrame(Frame& frame) override;

        void SetupDependencies() override;
    };
}  // namespace Observer