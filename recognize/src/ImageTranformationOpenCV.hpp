#include <opencv2/core/mat.hpp>
#include "ImageTransformation.hpp"
#include "opencv4/opencv2/opencv.hpp"
namespace Observer {
    template <>
    struct ImageTransformation<cv::Mat> {
        static void AddPad(cv::Mat& image, uint8_t top = 0, uint8_t bottom = 0,
                           uint8_t left = 0, uint8_t right = 0) {
            cv::copyMakeBorder(image, image, top, bottom, left, right, cv::BORDER_CONSTANT);
        }
        
        static cv::Mat HStackPadded(cv::Mat* images, uint8_t arraySize,
                                    uint8_t height) {
            int maxHeight = 0;
            cv::Mat res;

            assert(arraySize > 0);

            if (height == 0) {
                // calculate the ideal height (bigger)
                for (ushort i = 0; i < arraySize; i++) {
                    if (images[i].rows > maxHeight)
                        maxHeight = images[i].rows;
                }
            } else maxHeight = height;

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
                res = cv::Mat(640 * arraySize, 360, CV_8UC3, cv::Scalar(0, 0, 0)); 
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
                    if (images[i].cols > maxWidth)
                        maxWidth = images[i].cols;
                }
            } else maxWidth = width;

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
                hstacked.push_back(StackImages(&images[count], (arraySize - count), (arraySize - count)));
            }

            // pass 0 since its the width of two images Hstacked.
            return VStackPadded(&hstacked[0], hstacked.size(), 0);
        }

        static void RotateImage(cv::Mat& image, double angle) {
            cv::Point2f pc(image.cols / 2., image.rows / 2.);
            cv::Mat r = cv::getRotationMatrix2D(pc, angle, 1.0);
            cv::warpAffine(image, image, r, image.size());
        }
    };
}