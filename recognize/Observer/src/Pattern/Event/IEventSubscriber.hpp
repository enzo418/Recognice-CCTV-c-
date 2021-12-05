#pragma once

#include "../../Domain/Event/CameraEvent.hpp"
#include "../../Domain/Event/Event.hpp"
#include "../ObserverBasics.hpp"

namespace Observer {
    template <typename TFrame>
    class IEventSubscriber : public ISubscriber<Event, CameraEvent<TFrame>> {
        virtual void update(Event event,
                            CameraEvent<TFrame> rawCameraEvent) = 0;
    };
}  // namespace Observer