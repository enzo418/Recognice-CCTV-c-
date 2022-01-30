#pragma once

#include "../../Domain/Event/CameraEvent.hpp"
#include "../../Domain/Event/Event.hpp"
#include "../ObserverBasics.hpp"

namespace Observer {
    class IEventSubscriber : public ISubscriber<Event, CameraEvent> {
        virtual void update(Event event, CameraEvent rawCameraEvent) = 0;
    };
}  // namespace Observer