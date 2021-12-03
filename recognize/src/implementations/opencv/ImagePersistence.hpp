#include <opencv2/core/persistence.hpp>
#include <opencv2/imgcodecs.hpp>
#include "../../ImagePersistence.hpp"

#include "opencv4/opencv2/opencv.hpp"

namespace Observer {
    template <>
    struct ImagePersistence<cv::Mat> {
        static void SaveImage(const std::string& path, cv::Mat& image) {
            cv::imwrite(path, image);
        }

        static void ReadImage(const std::string& path, cv::Mat& imageOut) {
            imageOut = cv::imread(path);
        }
    };
}  // namespace Observer