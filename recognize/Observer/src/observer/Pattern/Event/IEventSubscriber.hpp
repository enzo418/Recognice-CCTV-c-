#pragma once

#include "observer/Domain/Event/CameraEvent.hpp"
#include "observer/Domain/Event/EventDescriptor.hpp"
#include "observer/Pattern/ObserverBasics.hpp"

namespace Observer {

    typedef Publisher<EventDescriptor&, std::shared_ptr<CameraEvent>>
        EventValidatorPublisher;

    typedef EventValidatorPublisher::Subscriber IEventValidatorSubscriber;
}  // namespace Observer