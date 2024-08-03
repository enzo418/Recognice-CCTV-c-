#include <chrono>
#include <cstdio>

#include "mdns.hpp"
#include "observer/AsyncInference/DetectorClient.hpp"
#include "observer/AsyncInference/types.hpp"
#include "observer/Implementation.hpp"
#include "observer/Instrumentation/Instrumentation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Timer.hpp"

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

    {  // Test constructor, destructor
        AsyncInference::DetectorClient* client =
            new AsyncInference::DetectorClient("");

        delete client;
    }

    /* ------------------- Call and print ------------------- */
    std::string serviceAddress = "0.0.0.0:3042";
    {
        mdns::MDNSClient mdnsClient("_darknet._tcp.local.");

        Observer::Timer<std::chrono::milliseconds> timer(true);

        std::future<std::optional<mdns::ServiceFound>> result =
            mdnsClient.findService(/*timeout*/ 1, /*wait_for_txt*/ true,
                                   /*wait_for_bothIP46*/ false);

        result.wait();
        if (result.valid()) {
            auto service = result.get();
            if (service.has_value() && service->ipv4_addr.has_value() &&
                service->port.has_value()) {
                OBSERVER_TRACE("Found mDNS service in {} ms",
                               timer.GetDuration());
                serviceAddress = service->ipv4_addr.value() + ":" +
                                 std::to_string(service->port.value());
            }
        }

        AsyncInference::DetectorClient client(serviceAddress);

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

    AsyncInference::DetectorClient client2(serviceAddress);

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

    /* ------ Level UP - Two detectors at the same time ----- */

    std::cout << "\nTrying two detectors\n";

    AsyncInference::DetectorClient client3(serviceAddress);
    AsyncInference::DetectorClient client4(serviceAddress);

    std::vector<Observer::Frame> images4 = {
        bus, bus, bus, bus, bus, bus, bus, bus, bus, bus, bus,
        bus, bus, bus, bus, bus, bus, bus, bus, bus, bus, bus,
        bus, bus, bus, bus, bus, bus, bus, bus, bus, bus};

    OBSERVER_ASSERT(client3.Connect(), "Failed to connect");
    OBSERVER_ASSERT(client4.Connect(), "Failed to connect");

    auto result_async1 = std::async(std::launch::async, [&client3, &images4]() {
        return client3.Detect(
            images4, [](const AsyncInference::SingleDetection& result) {
                assert(result.confidence > 0);
                assert(result.label.size() > 0);
            });
    });

    auto result_async2 = std::async(std::launch::async, [&client4, &images4]() {
        return client4.Detect(
            images4, [](const AsyncInference::SingleDetection& result) {
                assert(result.confidence > 0);
                assert(result.label.size() > 0);
            });
    });

    auto wait_results1 = result_async1.wait_for(std::chrono::seconds(10));
    auto wait_results2 = result_async2.wait_for(std::chrono::seconds(10));

    if (wait_results1 == std::future_status::timeout) {
        std::cout << "Timeout for async 1\n";
        return 1;
    }

    if (wait_results2 == std::future_status::timeout) {
        std::cout << "Timeout for async 2\n";
        return 1;
    }

    auto results1 = result_async1.get();
    auto results2 = result_async2.get();

    std::cout << "\n----All----\n";
    std::cout << "\tAsync 1 total: " << results1.size() << std::endl;
    std::cout << "\tAsync 2 total: " << results2.size() << std::endl;

    return 0;
}