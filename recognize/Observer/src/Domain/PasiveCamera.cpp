#include "PasiveCamera.hpp"

namespace Observer {
    PasiveCamera::PasiveCamera(CameraConfiguration* pConfiguration)
        : CameraObserver(pConfiguration) {}

    void PasiveCamera::ProcessFrame(Frame& frame) {}

    void PasiveCamera::SubscribeToCameraEvents(
        ICameraEventSubscriber* subscriber) {}

    void PasiveCamera::SubscribeToThresholdUpdate(
        IThresholdEventSubscriber* subscriber) {}
}  // namespace Observer