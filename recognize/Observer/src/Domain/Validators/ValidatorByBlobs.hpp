#pragma once

#include <iostream>
#include <vector>

#include "../../Blob/BlobDetectionConfiguration.hpp"
#include "../../Blob/BlobDetector/BlobDetector.hpp"
#include "../../Blob/Contours/ContoursDetector.hpp"
#include "ValidatorHandler.hpp"

namespace Observer {
    class ValidatorByBlobs : public ValidatorHandler {
       public:
        ValidatorByBlobs(const BlobDetectionConfiguration& pDetectorCfg);

       public:
        ValidationResult isValid(CameraEvent& request) override;

       private:
        ContoursDetector contoursDetector;
        BlobDetector blobDetector;
    };

}  // namespace Observer