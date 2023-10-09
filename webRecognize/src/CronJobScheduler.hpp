#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>

#include "DAL/INotificationRepository.hpp"
#include "DAL/NoLiteDB/VideoBufferRepositoryNLDB.hpp"
#include "observer/Functionality.hpp"
#include "observer/LockedFIFO.hpp"
#include "observer/Timer.hpp"
namespace Web {
    namespace {
        using Callback = std::function<void()>;
        using TimeType = uint32_t;
        using Timer = Observer::Timer<std::chrono::seconds>;
    }  // namespace

    struct CronJob {
        Callback run;
        const std::string name;
        const TimeType intervalSeconds;
        Timer timer;

        // will be true until it's executed for the first time
        bool runNow;
    };

    /**
     * @brief Cron job scheduler that works in the seconds range and runs the
     * jobs in a single thread.
     */
    class CronJobScheduler final : public Observer::Functionality {
       public:
        CronJobScheduler() = default;

        /**
         * @brief Adds a cron job
         *
         * @param name name of the job
         * @param intervalInSeconds
         * @param executeNow if true will be execute as soon as possible (once
         * the scheduler started and after)
         * @param callback
         */
        void Add(const std::string& name, uint32_t intervalInSeconds,
                 bool executeNow, Callback&& callback);

       private:
        void InternalStart() override;

       private:
        std::mutex jobsMutex;
        std::list<CronJob> jobs;

       private:
        typedef std::lock_guard<std::mutex> Guard;
    };
}  // namespace Web