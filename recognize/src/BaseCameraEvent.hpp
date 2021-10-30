#pragma once

#include "BaseObserverPattern.hpp"
#include "CameraConfiguration.hpp"

namespace Observer{
        using CameraEventPublisher = Publisher<CameraConfiguration*, RawCameraEvent>;

        class CameraEventSubscriber :
        public ISubscriber<CameraConfiguration*, RawCameraEvent> {
            void update(CameraConfiguration *cam, RawCameraEvent ev)
            override = 0;
        };
};