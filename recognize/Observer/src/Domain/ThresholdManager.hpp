#pragma once
#include <limits>

#include "../Pattern/ObserverBasics.hpp"
#include "../Timer.hpp"

namespace Observer {
    class ThresholdManager : public Publisher<double> {
       public:
        ThresholdManager(double minimumValue, double updateFrequency,
                         double increaseFactor);

        ThresholdManager& Add(double threshold) &;

        double GetAverage();

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
