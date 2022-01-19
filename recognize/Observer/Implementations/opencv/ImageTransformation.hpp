#include <opencv2/core/hal/interface.h>

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>

#include "../../src/ImageTransformation.hpp"
#include "../../src/Log/log.hpp"
#include "../../src/Utils/SpecialEnums.hpp"
#include "opencv4/opencv2/opencv.hpp"

namespace Observer {
    template <>
    struct ImageTransformation<cv::Mat> {
        static void AddPad(cv::Mat& image, int top = 0, int bottom = 0,
                           int left = 0, int right = 0) {
            cv::copyMakeBorder(image, image, top, bottom, left, right,
                               cv::BORDER_CONSTANT);
        }

        static cv::Mat HStackPadded(cv::Mat* images, int arraySize,
                                    int height) {
            int maxHeight = 0;
            cv::Mat res;

            assert(arraySize > 0);

            if (height == 0) {
                // calculate the ideal height (bigger)
                for (ushort i = 0; i < arraySize; i++) {
                    if (images[i].rows > maxHeight) maxHeight = images[i].rows;
                }
            } else
                maxHeight = height;

            for (ushort i = 0; i < arraySize; i++) {
                assert(images[i].dims <= 2);
                assert(images[i].type() == images[0].type());
                if (images[i].rows != maxHeight)
                    AddPad(images[i], 0, (maxHeight - images[i].rows));
            }

            try {
                cv::hconcat(images, arraySize, res);
            } catch (const std::exception& e) {
                OBSERVER_WARN(
                    "Unexpected error while stacking the images, mesage: "
                    "{}",
                    e.what());

                // return a black image of some size
                res =
                    cv::Mat(640 * arraySize, 360, CV_8UC3, cv::Scalar(0, 0, 0));
            }

            return res;
        }

        static cv::Mat VStackPadded(cv::Mat* images, int arraySize,
                                    int width = 0) {
            int maxWidth = 0;
            cv::Mat res;

            assert(arraySize > 0);

            if (width == 0) {
                // calculate the ideal width (bigger)
                for (int i = 0; i < arraySize; i++) {
                    if (images[i].cols > maxWidth) maxWidth = images[i].cols;
                }
            } else
                maxWidth = width;

            for (int i = 0; i < arraySize; i++) {
                if (images[i].cols != maxWidth)
                    AddPad(images[i], 0, 0, 0, (maxWidth - images[i].cols));
            }

            cv::vconcat(images, arraySize, res);
            return res;
        }

        static cv::Mat StackImages(cv::Mat* images, int arraySize,
                                   int maxHStack = 2) {
            std::vector<cv::Mat> hstacked;
            int count = 0;

            assert(maxHStack <= arraySize);

            if (arraySize == maxHStack) {
                return HStackPadded(&images[0], maxHStack, 0);
            }

            while (count <= (arraySize - maxHStack)) {
                hstacked.push_back(HStackPadded(&images[count], maxHStack, 0));
                count += maxHStack;
            }

            // (arraySize - count) = images left
            if ((arraySize - count) == 1) {
                hstacked.push_back(images[count]);
            } else if ((arraySize - count) > 1) {
                hstacked.push_back(StackImages(
                    &images[count], (arraySize - count), (arraySize - count)));
            }

            // pass 0 since its the width of two images Hstacked.
            return VStackPadded(&hstacked[0], hstacked.size(), 0);
        }

        static void RotateImage(cv::Mat& image, double angle) {
            cv::Point2f pc(image.cols / 2., image.rows / 2.);
            cv::Mat r = cv::getRotationMatrix2D(pc, angle, 1.0);
            cv::warpAffine(image, image, r, image.size(), cv::INTER_NEAREST);
        }

        static void AbsoluteDifference(cv::Mat& source1, cv::Mat& source2,
                                       cv::Mat& dest) {
            cv::absdiff(source1, source2, dest);
        }

        static void GaussianBlur(cv::Mat& source, cv::Mat& dst, int radius) {
            assert(radius > 0);
            assert(radius % 2 == 1);
            cv::GaussianBlur(source, dst, cv::Size(radius, radius), 10);
        }

        static void ToColorSpace(
            cv::Mat& source, cv::Mat& dst,
            int conversionType = ColorSpaceConversion::COLOR_RGB2GRAY) {
            switch (conversionType) {
                case COLOR_RGB2GRAY:
                    conversionType = cv::COLOR_RGB2GRAY;
                    break;
                case COLOR_GRAY2RGB:
                    conversionType = cv::COLOR_GRAY2RGB;
                    break;
                case COLOR_HLS2RGB:
                    conversionType = cv::COLOR_HLS2RGB;
                    break;
                case COLOR_RGB2HLS:
                    conversionType = cv::COLOR_RGB2HLS;
                    break;
            }

            cv::cvtColor(source, dst, conversionType);
        }

        static inline void CopyImage(cv::Mat& source, cv::Mat& dst) {
            source.copyTo(dst);
        }

        static void Resize(cv::Mat& source, cv::Mat& dst, const Size& size) {
            cv::Size sz(size.width, size.height);
            cv::resize(source, dst, sz);
        }

        static void Resize(cv::Mat& source, cv::Mat& dst,
                           const double scaleFactorX,
                           const double scaleFactorY) {
            cv::resize(source, dst, cv::Size(), scaleFactorX, scaleFactorY);
        }

        static void Threshold(cv::Mat& source, cv::Mat& dst, double threshold,
                              double max, int type) {
            int endtype = 0;

            if (has_flag(type, THRESHOLD_BINARY)) {
                set_flag(endtype, cv::THRESH_BINARY);
            }

            if (has_flag(type, THRESHOLD_BINARY_INV)) {
                set_flag(endtype, cv::THRESH_BINARY_INV);
            }

            if (has_flag(type, THRESHOLD_TRUNC)) {
                set_flag(endtype, cv::THRESH_TRUNC);
            }

            if (has_flag(type, THRESHOLD_TOZERO)) {
                set_flag(endtype, cv::THRESH_TOZERO);
            }

            if (has_flag(type, THRESHOLD_TOZERO_INV)) {
                set_flag(endtype, cv::THRESH_TOZERO_INV);
            }

            if (has_flag(type, THRESHOLD_TRIANGLE)) {
                set_flag(endtype, cv::THRESH_TRIANGLE);
            }

            cv::threshold(source, dst, threshold, max, endtype);
        }

        static int CountNonZero(cv::Mat& image) {
            return cv::countNonZero(image);
        }

        static inline void CropImage(cv::Mat& source, cv::Mat& dst,
                                     const Rect& roi) {
            dst = source(cv::Rect(roi.x, roi.y, roi.width, roi.height));
        }

        static inline cv::Mat BlackImage(cv::Mat* reference = nullptr,
                                         int channels = 3) {
            if (reference) {
                return cv::Mat::zeros(
                    cv::Size(reference->cols, reference->rows),
                    reference->type());
            } else {
                return cv::Mat::zeros(360, 640, CV_8UC3);
            }
        }

        static inline cv::Mat BlackImage(Size& size, int channels = 3) {
            int type;
            switch (type) {
                case 1:
                    type = CV_8UC1;
                    break;
                case 2:
                    type = CV_8UC2;
                    break;
                case 3:
                    type = CV_8UC3;
                    break;
                case 4:
                    type = CV_8UC4;
                    break;
            }

            return cv::Mat::zeros(size.height, size.width, type);
        }

        static Size GetSize(cv::Mat& image) {
            auto sz = image.size();
            return Size(sz.width, sz.height);
        }

        static inline void AddImages(cv::Mat& image1, cv::Mat& image2,
                                     cv::Mat& dst) {
            cv::add(image1, image2, dst);
        }

        static inline void EncodeImage(const std::string& ext, cv::Mat& image,
                                       int quality,
                                       std::vector<uchar>& buffer) {
            std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, quality};
            cv::imencode(ext, image, buffer, params);
        }
    };
}  // namespace Observer