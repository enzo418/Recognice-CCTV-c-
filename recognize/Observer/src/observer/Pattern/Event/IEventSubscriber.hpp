#pragma once

#include "observer/Domain/Event/CameraEvent.hpp"
#include "observer/Domain/Event/EventDescriptor.hpp"
#include "observer/Pattern/ObserverBasics.hpp"

namespace Observer {
    class IEventSubscriber : public ISubscriber<EventDescriptor, CameraEvent> {
        virtual void update(EventDescriptor event,
                            CameraEvent rawCameraEvent) = 0;
    };
}  // namespace Observer