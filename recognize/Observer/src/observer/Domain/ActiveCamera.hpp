#pragma once

#include "CameraObserver.hpp"

namespace Observer {
    class ActiveCamera : public CameraObserver {
       public:
        explicit ActiveCamera(CameraConfiguration* configuration);

       public:
        void SubscribeToCameraEvents(
            ICameraEventSubscriber* subscriber) override;

        void SubscribeToThresholdUpdate(
            IThresholdEventSubscriber* subscriber) override;

       protected:
        void ProcessFrame(Frame& frame) override;

        void SetupDependencies() override;

        void ChangeDetected();

        void NewVideoBuffer();

       private:
        // current change threshold
        double changeThreshold;

        FrameProcessor frameProcessor;

        std::unique_ptr<VideoBuffer> videoBufferForValidation;

        Publisher<CameraConfiguration*, CameraEvent> cameraEventsPublisher;

        ThresholdManager thresholdManager;

        /* Proxy allows us to give our subscribers not only the threshold,
         * but also the camera that updated the threshold.
         */
        class ThresholdEventPublisher : public ISubscriber<double> {
           public:
            explicit ThresholdEventPublisher(CameraConfiguration* pCfg)
                : cfg(pCfg) {}

            void subscribe(IThresholdEventSubscriber* subscriber) {
                this->thresholdPublisher.subscribe(subscriber);
            }

            void update(double thresh) override {
                this->thresholdPublisher.notifySubscribers(this->cfg, thresh);
            }

           private:
            CameraConfiguration* cfg;
            Publisher<CameraConfiguration*, double> thresholdPublisher;
        };

        // threshold publisher
        ThresholdEventPublisher thresholdPublisher;
    };
}  // namespace Observer