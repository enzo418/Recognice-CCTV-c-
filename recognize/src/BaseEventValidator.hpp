#pragma once

#include "BaseChainOfResponsibility.hpp"
#include "RawCameraEvent.hpp"
#include "Event.hpp"

namespace Observer {
    struct ValidationResult {
    public:
        ValidationResult() : valid(false)  {} // for use as default inside template

        explicit ValidationResult(bool pResult) : valid(pResult) {}

        ValidationResult(bool pResult, std::vector<std::string>& pMessages) :
                valid(pResult), messages(pMessages)
        { }

        Event& GetEvent() & {
            return this->event;
        }

        void SetValid(bool pValid) {
            this->valid = pValid;
        }

        bool IsValid() {
            return this->valid;
        }

        void AddMessage(std::string&& message) {
            this->messages.push_back(std::move(message));
        }

        std::vector<std::string>& GetMessages() & {
            return this->messages;
        }

    private:
        bool valid = false;
        std::vector<std::string> messages{};
        Event event;
    };

    using IValidatorHandler = Handler<ValidationResult, RawCameraEvent&>;

    /**
     * BaseConcreteHandler
     */
    class ValidatorHandler : public AbstractHandler<ValidationResult, RawCameraEvent&> {
    public:
        ValidationResult Handle(RawCameraEvent& request, ValidationResult &result) override {
            auto res = this->isValid(request, result);
            if (res.IsValid()) {
                return res;
            } else {
                return AbstractHandler<ValidationResult, RawCameraEvent&>::Handle(request, result);
            }
        }

        virtual ValidationResult isValid(RawCameraEvent& request, ValidationResult &result) = 0;
    };
}