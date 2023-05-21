#include "EventValidator.hpp"

#include <unordered_map>

#include "observer/AsyncInference/types.hpp"
#include "observer/Domain/Validators/ValidatorByNN.hpp"

namespace Observer {

    EventValidator::EventValidator(CameraConfiguration* pCameraCfg,
                                   SynchronizedIDProvider* pIdProvider) {
        this->cameraCfg = pCameraCfg;
        this->groupIdProvider = pIdProvider;

        /* ----------------- Validator by blobs ----------------- */
        ValidatorByBlobs* validatorByBlobs =
            new ValidatorByBlobs(this->cameraCfg->blobDetection);

        handlers.push_back(validatorByBlobs);

        /* ----------------- Validator by NN ----------------- */
        if (this->cameraCfg->objectDetectionValidatorConfig.enabled) {
            auto validatorByNN = new ValidatorByNN(
                this->cameraCfg->objectDetectionValidatorConfig);
            validatorByBlobs->SetNext(validatorByNN);
            handlers.push_back(validatorByNN);
        }

        /* ------------------- set first handler ---------------- */
        this->handler = handlers[0];
    }

    EventValidator::~EventValidator() {
        for (auto& h : this->handlers) {
            delete h;
        }

        this->handlers.clear();
    }

    void EventValidator::InternalStart() {
        while (this->running) {
            // wait until there is at least 1 item in the pool or exceeded
            // timeout, the last one is to avoid blocking forever if we
            // were stopped
            if (this->smpQueue.acquire_timeout<250>()) {
                // declare event and configuration
                CameraEvent rawCameraEvent;
                CameraConfiguration* cfg;

                // get next pool item
                std::tie(cfg, rawCameraEvent) = this->validationPool.pop();

                OBSERVER_TRACE(
                    "New event from camera '{}' received, analyzing it.",
                    cfg->name);

                // validate the event
                ValidationResult result;
                this->handler->Handle(rawCameraEvent, result);

                // send the event
                if (result.IsValid()) {
                    OBSERVER_TRACE("Event from camera '{}' was valid",
                                   cfg->name);

                    rawCameraEvent.SetGroupID(this->groupIdProvider->GetNext());

                    EventDescriptor& eventDescriptor = result.GetEvent();

                    /* ------------------- Classify blobs ------------------- */
                    auto& blobs = result.GetBlobs();
                    auto& objectDetections = result.GetDetections();

                    eventDescriptor.SetClassifications(
                        AssignObjectToBlob(blobs, objectDetections));
                    // -----------------------

                    // set camera name
                    eventDescriptor.SetCameraName(cfg->name);

                    // notify all subscribers
                    this->eventPublisher.notifySubscribers(eventDescriptor,
                                                           rawCameraEvent);
                } else {
                    OBSERVER_TRACE(
                        "Event from camera '{0}' was not valid due to {1}.",
                        cfg->name,
                        std::accumulate(
                            result.GetMessages().begin(),
                            result.GetMessages().end(), std::string(),
                            [](const std::string& a,
                               const std::string& b) -> std::string {
                                return a + (a.length() > 0 ? "," : "") + b;
                            }));
                }
            }
        }
    }

    void EventValidator::update(CameraConfiguration* cam, CameraEvent ev) {
        this->validationPool.push(std::make_pair(cam, std::move(ev)));

        // add 1 item to poll
        this->smpQueue.release();
    }

    void EventValidator::SubscribeToValidEvent(
        IEventValidatorSubscriber* subscriber, Priority priority) {
        this->eventPublisher.subscribe(subscriber, priority);
    }
}  // namespace Observer