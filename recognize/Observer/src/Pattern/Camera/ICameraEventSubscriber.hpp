#pragma once

#include "../ObserverBasics.hpp"
#include "../../Domain/Event/CameraEvent.hpp"
#include "../../Domain/Configuration/CameraConfiguration.hpp"

namespace Observer {
    template <typename TFrame>
    class ICameraEventSubscriber
        : public ISubscriber<CameraConfiguration*, CameraEvent<TFrame>> {
        void update(CameraConfiguration* cam,
                    CameraEvent<TFrame> ev) override = 0;
    };
};  // namespace Observer