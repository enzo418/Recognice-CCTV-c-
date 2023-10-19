#pragma once

#include "Streaming/Video/SourceOnDemand.hpp"
#include "observer/Pattern/ObserverBasics.hpp"

namespace Web::Streaming::Video {
    class ObserverOnDemand : public SourceOnDemand,
                             public ISubscriber<Observer::Frame> {
       public:
        ObserverOnDemand(int quality);

        void InternalStart() override;
        void PostStop() override;

        void update(Observer::Frame frame) override;

       protected:
        void EnsureOpen() override;
    };

}  // namespace Web::Streaming::Video