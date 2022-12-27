#include "VideoBufferTasksManager.hpp"

#include <utility>

#include "DTO/DTOBlob.hpp"
#include "Pattern/VideoBufferSubscriberPublisher.hpp"
#include "Serialization/JsonSerialization.hpp"
#include "Utils/VideoBuffer.hpp"
#include "observer/Domain/Configuration/CameraConfiguration.hpp"

namespace Web {

    VideoBufferTasksManager::VideoBufferTasksManager(
        DAL::VideoBufferRepositoryNLDB* pRepo,
        DAL::ConfigurationDAO* pConfigurationsDAO)
        : videoBufferRepository(pRepo), configurationsDAO(pConfigurationsDAO) {}

    void VideoBufferTasksManager::InternalStart() {
        this->running = true;

        while (this->running) {
            if (this->tasksSemaphore.acquire_timeout<250>()) {
                if (this->tasksGenerateVideo.size() > 0) {
                    this->ProcessTask(
                        std::move(this->tasksGenerateVideo.pop_front()));
                } else if (this->tasksDetection.size() > 0) {
                    this->ProcessTask(
                        std::move(this->tasksDetection.pop_front()));
                }
            }
        }
    }

    void VideoBufferTasksManager::SubscribeToTaskResult(
        VideoBufferSubscriber* subscriber) {
        this->videoBufferEventPublisher.subscribe(subscriber);
    }

    void VideoBufferTasksManager::ProcessTask(
        const GenerateVideoBufferTask& task) {
        Web::Utils::BufferData bufferData = Web::Utils::ReadBufferFromCamera(
            task.uri, task.delay, task.duration, task.resizeTo);

        std::string storedBufferPath = task.videoBufferTaskID + "_buffer.tiff";

        Web::Utils::SaveBuffer(bufferData.buffer, storedBufferPath);

        // update the value stored in the database
        videoBufferRepository->Update(task.videoBufferTaskID,
                                      {{"path", storedBufferPath},
                                       {"fps", bufferData.fps},
                                       {"state", "with_buffer"}});

        this->videoBufferEventPublisher.notifySubscribers(
            task.videoBufferTaskID, BufferEventType::BUFFER_READY, "");

        this->videoBufferEventPublisher.notifySubscribers(
            task.videoBufferTaskID, BufferEventType::UPDATED,
            videoBufferRepository->Get(task.videoBufferTaskID).dump());
        return;
    }

    void VideoBufferTasksManager::ProcessTask(
        DoDetectionVideoBufferTask&& task) {
        // get buffer id
        if (!this->videoBufferRepository->Exists(task.bufferID)) {
            this->videoBufferEventPublisher.notifySubscribers(
                task.bufferID, BufferEventType::CANCELED, "buffer not found");
            return;
        }

        auto path =
            this->videoBufferRepository->GetRawBufferPath(task.bufferID);

        if (!path) {
            this->videoBufferEventPublisher.notifySubscribers(
                task.bufferID, BufferEventType::CANCELED, "buffer not ready");
            return;
        }

        nldb::json buffer = this->videoBufferRepository->Get(task.bufferID);

        // get camera configuration
        nldb::json cameraJson;
        try {
            cameraJson =
                this->configurationsDAO->GetCamera(buffer["camera_id"]);
        } catch (...) {
            this->videoBufferEventPublisher.notifySubscribers(
                task.bufferID, BufferEventType::CANCELED, "camera not found");
            return;
        }

        Observer::CameraConfiguration camera = cameraJson;

        // get buffer from path
        auto frames = Web::Utils::ReadBufferFromFile(path.value());

        // do detection
        Utils::DetectionResults result =
            Web::Utils::DetectBlobs(camera, frames);

        std::string diffFramesPath = task.bufferID + "_buffer_diff.tiff";

        Web::Utils::SaveBuffer(result.diffFrames, diffFramesPath);

        std::vector<DTOBlob> dtoBlob;

        for (int i = 0; i < result.blobs.size(); i++) {
            dtoBlob.push_back(result.blobs[i]);
        }

        videoBufferRepository->Update(task.bufferID,
                                      {{"diffFramesPath", diffFramesPath},
                                       {"contours", result.contours},
                                       {"blobs", dtoBlob},
                                       {"state", "detected"}});

        this->videoBufferEventPublisher.notifySubscribers(
            task.bufferID, BufferEventType::DETECTION_DONE, "");

        this->videoBufferEventPublisher.notifySubscribers(
            task.bufferID, BufferEventType::UPDATED,
            videoBufferRepository->Get(task.bufferID).dump());
        return;
    }

    void VideoBufferTasksManager::AddTask(GenerateVideoBufferTask&& task) {
        this->tasksGenerateVideo.push_back(task);
        this->tasksSemaphore.release();
    }

    void VideoBufferTasksManager::AddTask(DoDetectionVideoBufferTask&& task) {
        this->tasksDetection.push_back(
            std::forward<DoDetectionVideoBufferTask&&>(task));
        this->tasksSemaphore.release();
    }

}  // namespace Web