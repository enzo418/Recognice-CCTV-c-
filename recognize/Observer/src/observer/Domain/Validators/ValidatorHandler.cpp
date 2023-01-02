#include "ValidatorHandler.hpp"

namespace Observer {
    ValidatorHandler::Result ValidatorHandler::Handle(CameraEvent& request) {
        auto res = this->isValid(request);
        if (res.IsValid()) {
            return res;
        } else {
            if (this->nextHandler) {
                return AbstractHandler<Result, CameraEvent&>::Handle(request);
            } else {
                return res;
            }
        }
    }
}  // namespace Observer