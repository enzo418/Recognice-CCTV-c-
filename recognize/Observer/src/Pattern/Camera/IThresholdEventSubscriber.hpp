#pragma once

#include "../../Domain/Configuration/CameraConfiguration.hpp"
#include "../ObserverBasics.hpp"

namespace Observer {
    class IThresholdEventSubscriber
        : public ISubscriber<CameraConfiguration*, double> {
        void update(CameraConfiguration*, double) override = 0;
    };
}  // namespace Observer