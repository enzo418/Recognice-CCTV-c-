#include "ThresholdManager.hpp"

#include "observer/Log/log.hpp"

namespace Observer {
    ThresholdManager::ThresholdManager() {
        this->thresholdAccumulator = 0;
        this->thresholdSamples = 0;
        this->threshold = std::numeric_limits<double>::max();
    }

    ThresholdManager& ThresholdManager::Add(double pThreshold) & {
        if (!this->timer.Started()) {
            this->timer.Start();
        }

        this->thresholdAccumulator += this->minimumValue + pThreshold;
        this->thresholdSamples += 1;

        if (this->timer.GetDuration() >= this->updateFrequency) {
            this->threshold = this->thresholdAccumulator /
                              this->thresholdSamples * this->increaseFactor;

            this->thresholdAccumulator = 0;
            this->thresholdSamples = 0;

            this->timer.GetDurationAndRestart();

            this->notifySubscribers(this->threshold);
        }

        return *this;
    }

    double ThresholdManager::GetAverage() { return this->threshold; }

    void ThresholdManager::Setup(double pMinimumValue, double pUpdateFrequency,
                                 double pIncreaseFactor) {
        this->thresholdAccumulator = 0;
        this->thresholdSamples = 0;
        this->threshold = std::numeric_limits<double>::max();

        this->minimumValue = pMinimumValue;
        this->updateFrequency = pUpdateFrequency;
        this->increaseFactor = pIncreaseFactor;
    }

}  // namespace Observer
