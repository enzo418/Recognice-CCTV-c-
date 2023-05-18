#pragma once

#include <iostream>
#include <vector>

#include "ValidatorHandler.hpp"
#include "observer/AsyncInference/DetectorClient.hpp"

namespace Observer {

    struct ValidatorConfig {
        std::string serverAddress;
        std::string cocoNamesFilePath;

        // detection
        float confidenceThreshold;
        std::unordered_map<std::string, int> minObjectCount;  // coco names
    };

    class ValidatorByNN final : public ValidatorHandler {
       public:
        ValidatorByNN(const ValidatorConfig& config);

       public:
        void isValid(CameraEvent& request, Result&) override;

       private:
        void ReadCocoNames();
        void ClearObjectCounter();

       private:
        ValidatorConfig config;
        AsyncInference::DetectorClient client;
        std::unordered_map<std::string, int> objectCount;
    };

}  // namespace Observer