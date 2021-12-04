#include "../../ImageTransformation.hpp"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#include "opencv4/opencv2/opencv.hpp"
namespace Observer {
    template <>
    struct ImageTransformation<cv::Mat> {
        static void AddPad(cv::Mat& image, uint8_t top = 0, uint8_t bottom = 0,
                           uint8_t left = 0, uint8_t right = 0) {
            cv::copyMakeBorder(image, image, top, bottom, left, right,
                               cv::BORDER_CONSTANT);
        }

        static cv::Mat HStackPadded(cv::Mat* images, uint8_t arraySize,
                                    uint8_t height) {
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
                // TODO: LOG?
                // std::cout << "Exception: " << e.what() << std::endl;
                // return a black image of some size
                res =
                    cv::Mat(640 * arraySize, 360, CV_8UC3, cv::Scalar(0, 0, 0));
            }

            return res;
        }

        static cv::Mat VStackPadded(cv::Mat* images, uint8_t arraySize,
                                    uint8_t width = 0) {
            int maxWidth = 0;
            cv::Mat res;

            assert(arraySize > 0);

            if (width == 0) {
                // calculate the ideal width (bigger)
                for (uint8_t i = 0; i < arraySize; i++) {
                    if (images[i].cols > maxWidth) maxWidth = images[i].cols;
                }
            } else
                maxWidth = width;

            for (uint8_t i = 0; i < arraySize; i++) {
                if (images[i].cols != maxWidth)
                    AddPad(images[i], 0, 0, 0, (maxWidth - images[i].cols));
            }

            cv::vconcat(images, arraySize, res);
            return res;
        }

        static cv::Mat StackImages(cv::Mat* images, uint8_t arraySize,
                                   uint8_t maxHStack = 2) {
            std::vector<cv::Mat> hstacked;
            uint8_t count = 0;

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
            cv::warpAffine(image, image, r, image.size());
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

        static void Threshold(cv::Mat& source, cv::Mat& dst, double threshold,
                              double max, int type) {
            switch (type) {
                case THRESHOLD_BINARY:
                    type = cv::THRESH_BINARY;
                    break;
                case THRESHOLD_BINARY_INV:
                    type = cv::THRESH_BINARY_INV;
                    break;
                case THRESHOLD_TRUNC:
                    type = cv::THRESH_TRUNC;
                    break;
                case THRESHOLD_TOZERO:
                    type = cv::THRESH_TOZERO;
                    break;
                case THRESHOLD_TOZERO_INV:
                    type = cv::THRESH_TOZERO_INV;
                    break;
            }

            cv::threshold(source, dst, threshold, max, type);
        }

        static int CountNonZero(cv::Mat& image) {
            return cv::countNonZero(image);
        }

        static inline void CropImage(cv::Mat& source, cv::Mat& dst, const Rect& roi) {
            dst = source(cv::Rect(roi.x, roi.y, roi.width, roi.height));
        }

        static inline cv::Mat BlackImage(cv::Mat* reference = nullptr) {
            if (reference) {
                return cv::Mat::zeros(cv::Size(reference->cols, reference->cols), reference->type());
            } else {
                return cv::Mat::zeros(360, 640, CV_8UC1);
            }
        }
    };
}  // namespace Observer