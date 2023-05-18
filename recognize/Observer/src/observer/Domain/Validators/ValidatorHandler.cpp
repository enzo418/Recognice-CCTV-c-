#include "ValidatorHandler.hpp"

namespace Observer {
    void ValidatorHandler::Handle(CameraEvent& request, Result& result) {
        // will return when all handlers are valid
        this->isValid(request, result);
        if (!result.IsValid() || !this->nextHandler) {
            return;
        } else {
            AbstractHandler<void, CameraEvent&, Result&>::Handle(request,
                                                                 result);
        }
    }
}  // namespace Observer