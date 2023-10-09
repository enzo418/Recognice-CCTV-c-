#pragma once

#include <queue>

#include "DAL/IConfigurationDAO.hpp"
#include "DAL/NoLiteDB/VideoBufferRepositoryNLDB.hpp"
#include "Pattern/VideoBufferSubscriberPublisher.hpp"
#include "Utils/VideoBuffer.hpp"
#include "observer/Functionality.hpp"
#include "observer/LockedFIFO.hpp"
#include "observer/Semaphore.hpp"
#include "observer/Size.hpp"

namespace Web {

    struct GenerateVideoBufferTask {
        const std::string videoBufferTaskID;
        const std::string cameraID;
        const std::string uri;
        const double duration;
        const double delay;
        const Observer::Size resizeTo;
    };

    struct DoDetectionVideoBufferTask {
        const std::string bufferID;
    };

    class VideoBufferTasksManager final : public Observer::Functionality {
       public:
        VideoBufferTasksManager(DAL::VideoBufferRepositoryNLDB* pRepo,
                                DAL::IConfigurationDAO* configurationsDAO);

        void AddTask(GenerateVideoBufferTask&& task);
        void AddTask(DoDetectionVideoBufferTask&& task);

        void SubscribeToTaskResult(VideoBufferSubscriber* subscriber);

       private:
        void InternalStart() override;

        void ProcessTask(const GenerateVideoBufferTask& task);
        void ProcessTask(DoDetectionVideoBufferTask&& task);

       private:
        Semaphore tasksSemaphore;

        Observer::BlockingFIFO<GenerateVideoBufferTask>::type
            tasksGenerateVideo;
        Observer::BlockingFIFO<DoDetectionVideoBufferTask>::type tasksDetection;

        DAL::IConfigurationDAO* configurationsDAO;
        DAL::VideoBufferRepositoryNLDB* videoBufferRepository;

        VideoBufferPublisher videoBufferEventPublisher;
    };
}  // namespace Web
