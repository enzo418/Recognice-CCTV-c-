#pragma once

#include "../ObserverBasics.hpp"
#include "../../Domain/Event/Event.hpp"
#include "../../Domain/Event/CameraEvent.hpp"

namespace Observer {
    template<typename TFrame>
    class IEventSubscriber : public ISubscriber<Event, CameraEvent<TFrame>> {
        virtual void update(Event event, CameraEvent<TFrame> rawCameraEvent) = 0;
    };
}