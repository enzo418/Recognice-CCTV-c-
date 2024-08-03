#include "ValidatorByNN.hpp"

#include <chrono>
#include <cmath>
#include <unordered_map>

#include "fstream"
#include "observer/AsyncInference/DetectorClient.hpp"
#include "observer/Instrumentation/Instrumentation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Utils/SpecialFunctions.hpp"

namespace Observer {
    ValidatorByNN::ValidatorByNN(const ValidatorConfig& pConfig)
        : config(pConfig),
          client(config.serverAddress),
          mdnsClient("_darknet._tcp.local.") {
        if (pConfig.maxFramesPerSecond <= 0) {
            throw std::invalid_argument(
                "maxFramesPerSecond must be greater than 0");
        }

        if (config.minObjectCount.empty()) {
            // throw std::invalid_argument("minObjectCount must be non-empty");
            config.minObjectCount.insert({"person", 2});
        }

        // Select the address of the server
        if (!client.Connect()) {
            OBSERVER_INFO("Failed to connect to inference server. Trying mDNS");

            std::string address = GetNewServerAddress();
            if (!address.empty()) {
                client.SetServerAddress(address);
            }
        }

        this->ReadCocoNames();
    }

    void ValidatorByNN::isValid(CameraEvent& request, Result& result) {
        OBSERVER_SCOPE("Validate event by NN");

        this->ClearObjectCounter();
        bool valid = false;

        AsyncInference::SendEveryNthFrame sendStrategy(
            ceil(request.GetFrameRate() / config.maxFramesPerSecond));

        {
            OBSERVER_SCOPE("Connect to inference server");
            int t = 3;
            while (t--) {
                if (client.Connect()) {
                    break;
                } else if (t == 1) {
                    OBSERVER_ERROR("Failed to connect");
                    result.SetValid(false);
                    result.AddMessages(
                        {"Couldn't connect to inference server"});

                    return;
                }

                OBSERVER_WARN("Failed to connect. Retrying");

                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }

        auto result_async = std::async(std::launch::async, [&]() {
            return client.Detect(
                request.GetFrames(),
                [this, &valid, &validationResult = result](
                    const AsyncInference::SingleDetection& result) {
                    if (!objectCount.contains(result.label)) {
                        OBSERVER_WARN("Unknown object: {}", result.label);
                    }

                    if (!config.minObjectCount.contains(result.label)) {
                        OBSERVER_TRACE("Skipping unwanted object: {} ({})",
                                       result.label, result.confidence);
                    } else {
                        if (result.confidence > config.confidenceThreshold) {
                            objectCount[result.label] += 1;
                        } else {
                            OBSERVER_INFO(
                                "Skipping low confidence object: {} ({})",
                                result.label, result.confidence);
                        }

                        if (objectCount[result.label] >
                            config.minObjectCount[result.label]) {
                            OBSERVER_INFO("Found enough ({}) {}s",
                                          objectCount[result.label],
                                          result.label);

                            client.Stop();
                            valid = true;
                            validationResult.AddMessages(
                                {"Stopping because of enough " + result.label});
                        }
                    }
                },
                &sendStrategy);
        });

        if (result_async.wait_for(std::chrono::seconds(10)) ==
            std::future_status::timeout) {
            OBSERVER_ERROR("Timeout while waiting for the result");
            result.SetValid(false);
            result.AddMessages({"Timeout while waiting for the result"});
            client.Stop();
            return;
        }

        if (valid) {
            result.SetValid(true);
            result.SetDetections(result_async.get());
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
        std::string filePath =
            Observer::SpecialFunctions::Paths::GetExecutableDirectory() /
            "assets/coco.names";
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Couldn't open " + filePath);
        }

        std::string line;
        while (std::getline(file, line)) {
            this->objectCount[line] = 0;
        }

        if (this->objectCount.empty()) {
            throw std::runtime_error("Couldn't read any object from " +
                                     filePath);
        }
    }

    std::string ValidatorByNN::GetNewServerAddress() {
        auto result = mdnsClient.findService(1, true, false);

        result.wait();
        if (result.valid()) {
            auto service = result.get();
            if (service.has_value() && service->ipv4_addr.has_value() &&
                service->port.has_value()) {
                OBSERVER_TRACE("Found mDNS service.");
                return service->ipv4_addr.value() + ":" +
                       std::to_string(service->port.value());
            }
        }

        return "";
    }
}  // namespace Observer