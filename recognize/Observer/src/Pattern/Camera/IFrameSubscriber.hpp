#pragma once

#include "../../IFrame.hpp"
#include "../ObserverBasics.hpp"

namespace Observer {
    class IFrameSubscriber : public ISubscriber<int, Frame> {
        virtual void update(int camerapos, Frame frame) = 0;
    };
}  // namespace Observer