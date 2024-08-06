#include "EventValidator.hpp"

#include <unordered_map>

#include "observer/AsyncInference/types.hpp"
#include "observer/Domain/Validators/ValidatorByNN.hpp"
#include "observer/Log/log.hpp"
#include "observer/Pattern/Validation/IValidatorHandler.hpp"

namespace Observer {

    class NullValidator final : public ValidatorHandler {
       public:
        void isValid(CameraEvent&, Result& result) { result.SetValid(true); }
    };

    EventValidator::EventValidator(CameraConfiguration* pCameraCfg,
                                   SynchronizedIDProvider* pIdProvider) {
        this->cameraCfg = pCameraCfg;
        this->groupIdProvider = pIdProvider;

        NullValidator* nullValidator = new NullValidator();
        this->handlers.push_back(nullValidator);

        Observer::IValidatorHandler* prev = nullValidator;

        /* ----------------- Validator by blobs ----------------- */
        if (this->cameraCfg->blobDetection.enabled) {
            ValidatorByBlobs* validatorByBlobs =
                new ValidatorByBlobs(this->cameraCfg->blobDetection);

            handlers.push_back(validatorByBlobs);

            if (prev != nullptr) {
                prev->SetNext(validatorByBlobs);
            }

            prev = validatorByBlobs;
        }

        /* ----------------- Validator by NN ----------------- */
        if (this->cameraCfg->objectDetectionValidatorConfig.enabled) {
            auto validatorByNN = new ValidatorByNN(
                this->cameraCfg->objectDetectionValidatorConfig, pCameraCfg);

            handlers.push_back(validatorByNN);

            if (prev != nullptr) {
                prev->SetNext(validatorByNN);
            }

            prev = validatorByNN;
        }

        if (!this->cameraCfg->blobDetection.enabled &&
            !this->cameraCfg->objectDetectionValidatorConfig.enabled) {
            OBSERVER_WARN("No validator was enabled for camera '{}'",
                          this->cameraCfg->name);
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
                std::shared_ptr<CameraEvent> rawCameraEvent;
                CameraConfiguration* cfg;

                // get next pool item
                std::tie(cfg, rawCameraEvent) = this->validationPool.pop();

                OBSERVER_TRACE(
                    "New event from camera '{}' received, analyzing it.",
                    cfg->name);

                // validate the event
                ValidationResult result;
                this->handler->Handle(*rawCameraEvent, result);

                // send the event
                if (result.IsValid()) {
                    OBSERVER_TRACE("Event from camera '{}' was valid",
                                   cfg->name);

                    rawCameraEvent->SetGroupID(
                        this->groupIdProvider->GetNext());

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

                    // notify ?
                }
            }
        }
    }

    void EventValidator::update(CameraConfiguration* cam,
                                std::shared_ptr<CameraEvent> ev) {
        this->validationPool.push(std::make_pair(cam, std::move(ev)));

        // add 1 item to poll
        this->smpQueue.release();
    }

    void EventValidator::SubscribeToValidEvent(
        IEventValidatorSubscriber* subscriber, Priority priority) {
        this->eventPublisher.subscribe(subscriber, priority);
    }
}  // namespace Observer