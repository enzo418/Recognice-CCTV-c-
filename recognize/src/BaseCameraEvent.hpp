#pragma once

#include "BaseObserverPattern.hpp"
#include "CameraConfiguration.hpp"
#include "RawCameraEvent.hpp"

namespace Observer {
    template <typename TFrame>
    class CameraEventSubscriber
        : public ISubscriber<CameraConfiguration*, RawCameraEvent<TFrame>> {
        void update(CameraConfiguration* cam,
                    RawCameraEvent<TFrame> ev) override = 0;
    };
};  // namespace Observer