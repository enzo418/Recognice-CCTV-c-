#pragma once
#include "Configuration.hpp"
#include "Notification.hpp"
#include "CameraObserver.hpp"

#include <opencv2/opencv.hpp>
#include <vector>
#include <thread>
#include <memory>

namespace Observer
{

    class ObserverCentral
    {
    public:
        ObserverCentral();

        bool Start(Configuration config);

        void StopCamera(std::string id);
        void StopAllCameras();

        void StartCamera(std::string id);
        void StartAllCameras();

        void StartPreview();
        void StopPreview();

    private:
        struct CameraThread {
            std::thread thread;
            std::shared_ptr<CameraObserver> camera;
        };

        std::vector<cv::Mat> outputFrames;
        Configuration config;

        std::vector<CameraThread> camerasThreads;

        CameraThread GetNewCameraThread(CameraConfiguration cfg);

        void interalStopCamera(CameraThread& camThread);
        void interalStartCamera(CameraConfiguration cfg);

    protected:
        void handleNewNotification(Notification notification);
        void handleThresholdUpdated(CameraObserver camera, float threshold);
        void handleNewFrame(cv::Mat frame, int cameraOrder);
    };

} // namespace Observer
