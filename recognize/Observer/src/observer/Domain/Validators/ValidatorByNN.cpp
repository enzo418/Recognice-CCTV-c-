#include "ValidatorByNN.hpp"

#include <unordered_map>

#include "fstream"
#include "observer/Log/log.hpp"

namespace Observer {
    ValidatorByNN::ValidatorByNN(const ValidatorConfig& pConfig)
        : config(pConfig), client(config.serverAddress) {}

    void ValidatorByNN::isValid(CameraEvent& request, Result& result) {
        this->ClearObjectCounter();
        bool valid = false;

        // OPTIMIZE: only send the frames where there is at least one blob

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
            });

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
        std::ifstream file(config.cocoNamesFilePath);
        std::string line;
        while (std::getline(file, line)) {
            this->objectCount[line] = 0;
        }
    }
}  // namespace Observer