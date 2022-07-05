#pragma once

#include "observer/IFrame.hpp"
#include "observer/Pattern/ObserverBasics.hpp"

namespace Observer {
    class IFrameSubscriber : public ISubscriber<int, Frame> {
        virtual void update(int camerapos, Frame frame) = 0;
    };
}  // namespace Observer