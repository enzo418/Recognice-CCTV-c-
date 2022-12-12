#include "ImageDisplay.hpp"

#include <opencv4/opencv2/core/mat.hpp>
#include <vector>

namespace Observer {
    /**
     * @brief Create a new Window
     *
     * @param name name of the new window
     */
    void ImageDisplay::CreateWindow(const std::string& name) {
        cv::namedWindow(name, cv::WINDOW_AUTOSIZE);
        cv::startWindowThread();
    }

    /**
     * @brief Show an image on a named window
     *
     * @param windowName name of the window where to show it
     * @param image
     */
    void ImageDisplay::ShowImage(const std::string& windowName, Frame& image) {
        cv::imshow(windowName, image.GetInternalFrame());
    }

    /**
     * @brief Destroy a window
     *
     * @param name name of the window
     */
    void ImageDisplay::DestroyWindow(const std::string& name) {
        cv::destroyWindow(name);
    }

    Frame ImageDisplay::StackImages(Frame* wrappedImages, uint8_t arraySize,
                                    uint8_t maxHStack) {
        std::vector<cv::Mat> images(arraySize);

        // Transform cv::Mat wrapper vector to cv::Mat vector.
        // Can we just store all the pointers to the image like
        // std::vector<cv::Mat*> images = for wrappedimages as w return
        // &w.Internal()?
        // no, since the internal stack uses pointers to speed up the process.
        // So we need to copy them.

        for (int i = 0; i < arraySize; i++) {
            // pray that the internal type is copying the image on = op!
            images[i] = wrappedImages[i].GetInternalFrame();
        }

        return Frame(InternalStackImages(&images[0], arraySize, maxHStack));
    }

    /**
     * @brief Stack images Horizontally
     *
     * @param images Array of images to stack
     * @param arraySize Amount of images to concat
     * @param height Height of each image in the array.
     * 0 to automatically calculate it.
     * @return Frame
     */
    cv::Mat ImageDisplay::HStackPadded(cv::Mat* images, uint8_t arraySize,
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
            // return a black image of some size
            res = cv::Mat(640 * arraySize, 360, CV_8UC3, cv::Scalar(0, 0, 0));
        }

        return res;
    }

    /**
     * @brief Stack images Vertically
     *
     * @param images Array of images to stack
     * @param arraySize Amount of images to concat
     * @param width Width of each image in the array. 0 to automatically
     * calculate it.
     * @return Frame
     */
    cv::Mat ImageDisplay::VStackPadded(cv::Mat* images, uint8_t arraySize,
                                       uint8_t width) {
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

    /**
     * @brief Stack `arraySize/maxHStack` rows vertically.
     * The rows are created from first image to last,
     * where the number of columns is `maxHStack`.
     *
     * @param images Array of images to stack
     * @param arraySize Number of images to stack
     * @param maxHStack Number of images to stack horizontally on each row
     * @return Frame
     */
    cv::Mat ImageDisplay::InternalStackImages(cv::Mat* images,
                                              uint8_t arraySize,
                                              uint8_t maxHStack) {
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
            hstacked.push_back(InternalStackImages(
                &images[count], (arraySize - count), (arraySize - count)));
        }

        // pass 0 since its the width of two images Hstacked.
        return VStackPadded(&hstacked[0], hstacked.size(), 0);
    }

    /**
     * @brief Add pad to iamge
     *
     * @param image image to add the pad
     * @param top Pad to add on top of the image
     * @param bottom Pad to add below the image
     * @param left Pad to add in the left side of the image
     * @param right Pad to add in the right side of the image
     */
    void ImageDisplay::AddPad(cv::Mat& image, uint8_t top, uint8_t bottom,
                              uint8_t left, uint8_t right) {
        cv::copyMakeBorder(image, image, top, bottom, left, right,
                           cv::BORDER_CONSTANT);
    }

    ImageDisplay& ImageDisplay::Get() {
        static ImageDisplay instance;
        return instance;
    }
}  // namespace Observer