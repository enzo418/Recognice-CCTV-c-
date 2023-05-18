#include "observer/AsyncInference/DetectorClient.hpp"
#include "observer/Implementation.hpp"

int main() {
    AsyncInference::DetectorClient client("0.0.0.0:50051");

    // -----------------------------
    auto names = client.GetModelNames();

    std::cout << "Model names:\n";
    for (const auto& name : names) {
        std::cout << name << std::endl;
    }

    // -----------------------------

    Observer::Frame bus(cv::imread("bus.jpg"));

    std::vector<Observer::Frame> images = {bus, bus, bus, bus};

    std::vector<AsyncInference::ImageDetections> results = client.Detect(
        images, [](const AsyncInference::ImageDetections& result) {
            std::cout << "Result: " << result.image_index << std::endl;

            assert(result.detections.size() > 0);

            for (const auto& detection : result.detections) {
                std::cout << "\tLabel: " << detection.label << std::endl;
            }
        });

    std::cout << "\n----All----\n";

    for (const auto& result : results) {
        std::cout << "Result: " << result.image_index << std::endl;

        for (const auto& detection : result.detections) {
            std::cout << "\tLabel: " << detection.label << std::endl;
        }
    }

    return 0;
}