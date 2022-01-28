#pragma once

#include <numeric>

#include "../Functionality.hpp"
#include "../IFrame.hpp"
#include "../Log/log.hpp"
#include "../Pattern/Camera/ICameraEventSubscriber.hpp"
#include "../Pattern/Event/IEventSubscriber.hpp"
#include "../Pattern/Validation/IValidatorHandler.hpp"
#include "../Semaphore.hpp"
#include "../SimpleBlockingQueue.hpp"
#include "Event/Event.hpp"
#include "Validators/ValidatorByBlobs.hpp"
#include "Validators/ValidatorHandler.hpp"

namespace Observer {
    class EventValidator : public ICameraEventSubscriber, public Functionality {
       public:
        EventValidator(CameraConfiguration* cfg);

        void Add(CameraConfiguration* cfg, CameraEvent ev);

        void SubscribeToEventValidationDone(IEventSubscriber* subscriber);

        void update(CameraConfiguration* cfg, CameraEvent ev) override;

        ~EventValidator();

       protected:
        void InternalStart() override;

       private:
        CameraConfiguration* cameraCfg;

        Semaphore smpQueue;

        IValidatorHandler* handler;

        std::vector<IValidatorHandler*> handlers;

        SimpleBlockingQueue<std::pair<CameraConfiguration*, CameraEvent>>
            validationPool;

        Publisher<Event, CameraEvent> eventPublisher;
    };
}  // namespace Observer