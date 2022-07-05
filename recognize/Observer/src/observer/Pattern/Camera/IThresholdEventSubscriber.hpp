#pragma once

#include "observer/Domain/Configuration/CameraConfiguration.hpp"
#include "observer/Pattern/ObserverBasics.hpp"

namespace Observer {
    class IThresholdEventSubscriber
        : public ISubscriber<CameraConfiguration*, double> {
        void update(CameraConfiguration*, double) override = 0;
    };
}  // namespace Observer