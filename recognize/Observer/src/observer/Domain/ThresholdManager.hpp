#pragma once
#include <limits>

#include "observer/Pattern/ObserverBasics.hpp"
#include "observer/Timer.hpp"

namespace Observer {
    class ThresholdManager : public Publisher<double> {
       public:
        ThresholdManager();

        ThresholdManager& Add(double threshold) &;

        double GetAverage();

        void Setup(double minimumValue, double updateFrequency,
                   double increaseFactor);

       private:
        double threshold;
        double thresholdAccumulator;
        double thresholdSamples;

        double updateFrequency;
        double increaseFactor;

        double minimumValue;

        Timer<std::chrono::seconds> timer;
    };
}  // namespace Observer
