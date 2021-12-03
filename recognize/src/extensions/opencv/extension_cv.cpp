#include "extension_cv.hpp"

namespace cv {
    bool operator==(const cv::Size& s1, const cv::Size& s2) {
        return std::tie(s1.height, s1.width) == std::tie(s1.height, s1.width);
    }
}  // namespace cv