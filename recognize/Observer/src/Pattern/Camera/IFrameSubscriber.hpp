#pragma once

#include "../ObserverBasics.hpp"

namespace Observer {
    template <typename TFrame>
    class IFrameSubscriber : public ISubscriber<int, TFrame> {
        virtual void update(int camerapos, TFrame frame) = 0;
    };
}  // namespace Observer