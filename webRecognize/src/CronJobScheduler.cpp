#include "CronJobScheduler.hpp"

#include <chrono>
#include <limits>
#include <thread>

#include "observer/Log/log.hpp"
#include "observer/Timer.hpp"

namespace Web {
    void CronJobScheduler::Add(const std::string& name,
                               uint32_t intervalInSeconds, bool executeNow,
                               Callback&& callback) {
        Guard g(this->jobsMutex);
        this->jobs.push_back(CronJob {.run = std::move(callback),
                                      .name = name,
                                      .intervalSeconds = intervalInSeconds,
                                      .timer = Timer(true),
                                      .runNow = executeNow});
    }

    void CronJobScheduler::InternalStart() {
        Timer timerSinceSetLeft(true);

        while (this->running) {
            this->jobsMutex.lock();
            timerSinceSetLeft.Restart();

            int64_t sleepUntilNext = 0;
            bool runAgain = false;
            int64_t left = 0;

            while (sleepUntilNext <= 0) {
                for (auto& job : jobs) {
                    const double d = job.timer.GetDuration();
                    left = job.intervalSeconds - d;
                    if (left <= 0 || job.runNow) {
                        OBSERVER_TRACE("Running cron job '{}'", job.name);

                        job.run();
                        job.timer.Restart();
                        job.runNow = false;
                    } else {
                        if (sleepUntilNext != 0 &&
                            sleepUntilNext - timerSinceSetLeft.GetDuration() <
                                0) {
                            // a job took too long, must restart now
                            runAgain = true;
                            sleepUntilNext = 0;
                        }

                        if (!runAgain) {
                            sleepUntilNext = left;
                            timerSinceSetLeft.Restart();
                        }
                    }
                }

                sleepUntilNext =
                    sleepUntilNext - timerSinceSetLeft.GetDurationAndRestart();
            }

            this->jobsMutex.unlock();

            std::this_thread::sleep_for(std::chrono::seconds(sleepUntilNext));
        }
    }

}  // namespace Web