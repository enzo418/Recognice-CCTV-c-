#pragma once

#include "BaseChainOfResponsibility.hpp"
#include "Event.hpp"
#include "RawCameraEvent.hpp"

namespace Observer {

    template <typename TFrame>
    struct ValidationResult {
       public:
        ValidationResult()
            : valid(false) {}  // for use as default inside template

        explicit ValidationResult(bool pResult) : valid(pResult) {}

        ValidationResult(bool pResult, std::vector<std::string>& pMessages)
            : valid(pResult), messages(pMessages) {}

        Event& GetEvent() & { return this->event; }

        void SetValid(bool pValid) { this->valid = pValid; }

        bool IsValid() { return this->valid; }

        void AddMessage(std::string&& message) {
            this->messages.push_back(std::move(message));
        }

        std::vector<std::string>& GetMessages() & { return this->messages; }

       private:
        bool valid = false;
        std::vector<std::string> messages {};
        Event event;
    };

    /**
     * BaseConcreteHandler
     */
    template <typename TFrame>
    class ValidatorHandler : public AbstractHandler<ValidationResult<TFrame>,
                                                    RawCameraEvent<TFrame>&> {
       public:
        using IValidatorHandler =
            Handler<ValidationResult<TFrame>, RawCameraEvent<TFrame>&>;

        ValidationResult<TFrame> Handle(
            RawCameraEvent<TFrame>& request,
            ValidationResult<TFrame>& result) override {
            auto res = this->isValid(request, result);
            if (res.IsValid()) {
                return res;
            } else {
                return AbstractHandler<ValidationResult<TFrame>,
                                       RawCameraEvent<TFrame>&>::Handle(request,
                                                                        result);
            }
        }

        virtual ValidationResult<TFrame> isValid(
            RawCameraEvent<TFrame>& request,
            ValidationResult<TFrame>& result) = 0;
    };
}  // namespace Observer