#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "ValidatorHandler.hpp"
#include "observer/AsyncInference/DetectorClient.hpp"

namespace Observer {

    struct ValidatorConfig {
        bool enabled {false};

        std::string serverAddress;

        // detection
        float confidenceThreshold {0.4};
        std::map<std::string, int> minObjectCount {
            {"person", 2}};  // coco names

        // send strategy
        float maxFramesPerSecond {1};  // 1 = 1 fps

        bool operator==(const ValidatorConfig&) const = default;
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