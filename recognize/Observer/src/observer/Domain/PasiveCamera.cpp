#include "PasiveCamera.hpp"

namespace Observer {
    PassiveCamera::PassiveCamera(CameraConfiguration* pConfiguration)
        : CameraObserver(pConfiguration) {}

    void PassiveCamera::ProcessFrame(Frame&) {}

    void PassiveCamera::SubscribeToCameraEvents(ICameraEventSubscriber*) {}

    void PassiveCamera::SubscribeToThresholdUpdate(IThresholdEventSubscriber*) {
    }

    void PassiveCamera::SetupDependencies() {}
}  // namespace Observer