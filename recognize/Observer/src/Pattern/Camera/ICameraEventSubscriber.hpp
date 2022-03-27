#pragma once

#include "../../Domain/Configuration/CameraConfiguration.hpp"
#include "../../Domain/Event/CameraEvent.hpp"
#include "../../IFrame.hpp"
#include "../ObserverBasics.hpp"

namespace Observer {
    class ICameraEventSubscriber
        : public ISubscriber<CameraConfiguration*, CameraEvent> {
        void update(CameraConfiguration* cam, CameraEvent ev) override = 0;
    };
};  // namespace Observer