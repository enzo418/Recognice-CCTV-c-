#pragma once

#include <iostream>
#include <vector>

#include "ValidatorHandler.hpp"
#include "observer/Blob/BlobDetectionConfiguration.hpp"
#include "observer/Blob/BlobDetector/BlobDetector.hpp"
#include "observer/Blob/Contours/ContoursDetector.hpp"

namespace Observer {
    class ValidatorByBlobs final : public ValidatorHandler {
       public:
        ValidatorByBlobs(const BlobDetectionConfiguration& pDetectorCfg);

       public:
        void isValid(CameraEvent& request, Result& result) override;

       private:
        ContoursDetector contoursDetector;
        BlobDetector blobDetector;
    };

}  // namespace Observer