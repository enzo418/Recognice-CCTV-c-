#pragma once

#include "observer/Domain/Event/CameraEvent.hpp"
#include "observer/Domain/Event/Event.hpp"
#include "observer/Pattern/ObserverBasics.hpp"

namespace Observer {
    class IEventSubscriber : public ISubscriber<Event, CameraEvent> {
        virtual void update(Event event, CameraEvent rawCameraEvent) = 0;
    };
}  // namespace Observer