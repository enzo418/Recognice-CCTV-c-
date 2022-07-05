#pragma once

#include "observer/Domain/Configuration/CameraConfiguration.hpp"
#include "observer/Domain/Event/CameraEvent.hpp"
#include "observer/IFrame.hpp"
#include "observer/Pattern/ObserverBasics.hpp"

namespace Observer {
    class ICameraEventSubscriber
        : public ISubscriber<CameraConfiguration*, CameraEvent> {
        void update(CameraConfiguration* cam, CameraEvent ev) override = 0;
    };
};  // namespace Observer