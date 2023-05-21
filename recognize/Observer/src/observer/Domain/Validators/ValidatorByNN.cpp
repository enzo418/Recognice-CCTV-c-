#include "ValidatorByNN.hpp"

#include <cmath>
#include <unordered_map>

#include "fstream"
#include "observer/AsyncInference/DetectorClient.hpp"
#include "observer/Instrumentation/Instrumentation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Utils/SpecialFunctions.hpp"

namespace Observer {
    ValidatorByNN::ValidatorByNN(const ValidatorConfig& pConfig)
        : config(pConfig), client(config.serverAddress) {
        if (pConfig.maxFramesPerSecond <= 0) {
            throw std::invalid_argument(
                "maxFramesPerSecond must be greater than 0");
        }
    }

    void ValidatorByNN::isValid(CameraEvent& request, Result& result) {
        OBSERVER_SCOPE("Validate event by NN");

        this->ClearObjectCounter();
        bool valid = false;

        AsyncInference::SendEveryNthFrame sendStrategy(
            ceil(request.GetFrameRate() * config.maxFramesPerSecond));

        auto imagesDetections = client.Detect(
            request.GetFrames(),
            [this, &valid, &validationResult = result](
                AsyncInference::ImageDetections& detections) {
                for (auto& result : detections.detections) {
                    if (!objectCount.contains(result.label)) {
                        OBSERVER_WARN("Unknown object: {}", result.label);
                        continue;
                    }

                    if (result.confidence > config.confidenceThreshold) {
                        objectCount[result.label] += 1;
                    }

                    if (objectCount[result.label] >
                        config.minObjectCount[result.label]) {
                        client.Stop();
                        valid = true;
                        validationResult.AddMessages(
                            {"Stopping because of enough " + result.label});
                        break;
                    }
                }
            },
            &sendStrategy);

        if (valid) {
            result.SetValid(true);
            result.SetDetections(std::move(imagesDetections));
        } else {
            result.SetValid(false);
            result.AddMessages({"Couldn't find significant objects."});
        }
    }

    void ValidatorByNN::ClearObjectCounter() {
        for (auto& [object, _] : this->objectCount) {
            this->objectCount[object] = 0;
        }
    }

    void ValidatorByNN::ReadCocoNames() {
        std::ifstream file(
            Observer::SpecialFunctions::Paths::GetExecutableDirectory() /
            "assets/coco.names");
        if (!file.is_open()) {
            throw std::runtime_error("Couldn't open assets/coco.names");
        }

        std::string line;
        while (std::getline(file, line)) {
            this->objectCount[line] = 0;
        }
    }
}  // namespace Observer