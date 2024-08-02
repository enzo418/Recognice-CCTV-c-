#include <chrono>
#include <cstdio>

#include "observer/AsyncInference/DetectorClient.hpp"
#include "observer/AsyncInference/types.hpp"
#include "observer/Implementation.hpp"
#include "observer/Instrumentation/Instrumentation.hpp"
#include "observer/Log/log.hpp"

int main() {
    OBSERVER_INIT_INSTRUMENTATION();
    OBSERVER_DECLARE_THREAD("Main");

    // initialize logger
    Observer::LogManager::Initialize();
    OBSERVER_INFO("Hi");

    // -----------------------------

    Observer::Frame bus(cv::imread("bus.jpg"));

    // -----------------------------
    // auto names = client.GetModelNames();

    // std::cout << "Model names:\n";
    // for (const auto& name : names) {
    //     std::cout << name << std::endl;
    // }

    /* ------------------- Call and print ------------------- */
    {
        AsyncInference::DetectorClient client("0.0.0.0:3042");

        std::vector<Observer::Frame> images = {bus, bus, bus, bus,
                                               bus, bus, bus, bus};

        std::cout << "\n----Flash----\n";

        int t = 3;
        while (t--) {
            if (client.Connect()) {
                break;
            } else if (t == 1) {
                std::cout << "Failed to connect\n";
                return 1;
            }

            OBSERVER_WARN("Failed to connect. Retrying");

            std::this_thread::sleep_for(std::chrono::seconds(10));
        }

        std::vector<AsyncInference::ImageDetections> results = client.Detect(
            images, [](const AsyncInference::SingleDetection& result) {
                assert(result.confidence > 0);
                assert(result.label.size() > 0);

                printf("\tLabel: %-4s Confidence: %3.2f\n",
                       result.label.c_str(), result.confidence);
            });

        std::cout << "\n----All----\n";

        for (const auto& result : results) {
            std::cout << "Result: " << result.image_index << std::endl;

            for (const auto& detection : result.detections) {
                printf("\tLabel: %-4s Confidence: %3.2f\n",
                       detection.label.c_str(), detection.confidence);
            }
        }

        client.Disconnect();

        /* ---------------- Call and print again ---------------- */

        std::cout << "\nTrying second time\n";

        std::vector<Observer::Frame> images2 = {bus, bus, bus, bus, bus, bus,
                                                bus, bus, bus, bus, bus, bus,
                                                bus, bus, bus, bus};

        OBSERVER_ASSERT(client.Connect(), "Failed to connect");

        results = client.Detect(
            images2, [](const AsyncInference::SingleDetection& result) {
                assert(result.confidence > 0);
                assert(result.label.size() > 0);
            });

        std::cout << "\n----All----\n";

        for (const auto& result : results) {
            std::cout << "Result: " << result.image_index << std::endl;

            for (const auto& detection : result.detections) {
                printf("\tLabel: %-4s Confidence: %3.2f\n",
                       detection.label.c_str(), detection.confidence);
            }
        }
    }

    /* ------------- Destroy, new call and print ------------ */

    std::cout << "\nTrying third time\n";

    AsyncInference::DetectorClient client2("0.0.0.0:3042");

    std::vector<Observer::Frame> images3 = {bus, bus, bus, bus, bus, bus,
                                            bus, bus, bus, bus, bus, bus,
                                            bus, bus, bus, bus};

    OBSERVER_ASSERT(client2.Connect(), "Failed to connect");

    auto results = client2.Detect(
        images3, [](const AsyncInference::SingleDetection& result) {
            assert(result.confidence > 0);
            assert(result.label.size() > 0);
        });

    std::cout << "\n----All----\n";

    for (const auto& result : results) {
        std::cout << "Result: " << result.image_index << std::endl;

        for (const auto& detection : result.detections) {
            printf("\tLabel: %-4s Confidence: %3.2f\n", detection.label.c_str(),
                   detection.confidence);
        }
    }

    return 0;
}