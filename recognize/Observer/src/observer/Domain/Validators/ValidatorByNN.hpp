#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "ValidatorHandler.hpp"
#include "mdns.hpp"
#include "observer/AsyncInference/DetectorClient.hpp"

namespace Observer {

    struct ValidatorConfig {
        bool enabled {false};

        std::string serverAddress;

        // detection
        float confidenceThreshold {0.4};
        std::map<std::string, int> minObjectCount {
            {"person", 1}};  // coco names

        // send strategy
        float maxFramesPerSecond {1};  // 1 = 1 fps

        // rotation of the camera in degrees
        // same as the one in the camera configuration
        bool applyRotation;

        // Mask parts of the image out
        // relative to the camera resolution
        // same as the one in the camera configuration
        bool applyMasks;

        bool operator==(const ValidatorConfig&) const = default;
    };

    class ValidatorByNN final : public ValidatorHandler {
       public:
        ValidatorByNN(const ValidatorConfig& config,
                      struct CameraConfiguration* cameraCfg);

       public:
        void isValid(CameraEvent& request, Result&) override;

       private:
        void ReadCocoNames();
        void ClearObjectCounter();

        std::string GetNewServerAddress();

        void SetupMask(Size);

       private:
        ValidatorConfig config;
        struct CameraConfiguration* cameraCfg;
        AsyncInference::DetectorClient client;
        std::unordered_map<std::string, int> objectCount;

        mdns::MDNSClient mdnsClient;

        Frame mask;
    };

}  // namespace Observer