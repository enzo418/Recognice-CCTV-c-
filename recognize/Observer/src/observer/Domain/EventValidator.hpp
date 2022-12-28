#pragma once

#include <numeric>

#include "Event/EventDescriptor.hpp"
#include "Validators/ValidatorByBlobs.hpp"
#include "Validators/ValidatorHandler.hpp"
#include "observer/Domain/SynchronizedIDProvider.hpp"
#include "observer/Functionality.hpp"
#include "observer/IFrame.hpp"
#include "observer/Log/log.hpp"
#include "observer/Pattern/Camera/ICameraEventSubscriber.hpp"
#include "observer/Pattern/Event/IEventSubscriber.hpp"
#include "observer/Pattern/Validation/IValidatorHandler.hpp"
#include "observer/Semaphore.hpp"
#include "observer/SimpleBlockingQueue.hpp"

namespace Observer {
    class EventValidator : public ICameraEventSubscriber, public Functionality {
       public:
        EventValidator(CameraConfiguration* cfg,
                       SynchronizedIDProvider* groupIdProvider);

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

        Publisher<EventDescriptor, CameraEvent> eventPublisher;

        SynchronizedIDProvider* groupIdProvider;
    };
}  // namespace Observer