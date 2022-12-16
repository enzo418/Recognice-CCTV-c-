#include "PasiveCamera.hpp"

namespace Observer {
    PassiveCamera::PassiveCamera(CameraConfiguration* pConfiguration)
        : CameraObserver(pConfiguration) {}

    void PassiveCamera::ProcessFrame(Frame& frame) {}

    void PassiveCamera::SubscribeToCameraEvents(
        ICameraEventSubscriber* subscriber) {}

    void PassiveCamera::SubscribeToThresholdUpdate(
        IThresholdEventSubscriber* subscriber) {}

    void PassiveCamera::SetupDependencies() {}
}  // namespace Observer