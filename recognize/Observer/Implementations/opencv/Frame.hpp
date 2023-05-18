#pragma once

#include <opencv2/opencv.hpp>

#include "observer/IFrame.hpp"
#include "observer/Utils/SpecialEnums.hpp"

namespace Observer {
    class Frame final : public IFrame {
       public:
        // internal type
        typedef cv::Mat IType;

       public:
        Frame();

        Frame(IType&& frame);

        /**
         * @brief Construct a new Frame object with a size and number of
         * channels
         *
         * @param size
         * @param numberChannels
         */
        Frame(const Size& size, int numberChannels);

       public:
        /**
         * @brief Creates a black image with the same size and type of
         * this one.
         *
         * @return Frame
         */
        Frame GetBlackImage() override;

       protected:
        /**
         * @brief Creates a black image.
         *
         * @param size
         * @param numberChannels
         * @return Frame
         */
        static Frame::IType GetBlackImage(const Size& size, int numberChannels);

       public:
        IType& GetInternalFrame();

        Frame Clone() override;

        bool IsEmpty() override;

        /**
         * @brief Get the number of channels
         *
         * @return int
         */
        int GetNumberChannels();

        /**
         * @brief Rotates a image
         *
         * @param angle angle, on degrees
         */
        void RotateImage(double angle) override;

        /**
         * @brief Resize the image
         *
         * @param size
         */
        void Resize(const Size& size) override;

        /**
         * @brief Resize the image
         *
         * @param scaleFactorX
         * @param scaleFactorY
         */
        void Resize(const double scaleFactorX,
                    const double scaleFactorY) override;

        /**
         * @brief Copy the image
         *
         * @param dst
         */
        void CopyTo(Frame& dst) override;

        /**
         * @brief Calculates the difference between this image and `image2`.
         * This and image2 need to have the same number of channels.
         *
         * @param image2
         * @return Frame
         */
        Frame AbsoluteDifference(Frame& image2) override;

        /**
         * @brief Crop a image, no data is copied.
         * The destionation image pointer will be pointing to the the sub-array
         * associated with the specified roi.
         *
         * @param roi region of interest
         */
        void CropImage(const Rect& roi) override;

        /**
         * @brief Blurs an image using a Gaussian filter
         *
         * @param radius
         */
        void GaussianBlur(int radius) override;

        /**
         * @brief Converts an image from one color space to another
         *
         * @param source source image
         * @param dst destination image
         * @param conversionType space conversion (ColorSpaceConversion)
         */
        void ToColorSpace(
            int conversionType = ColorSpaceConversion::COLOR_RGB2GRAY) override;

        /**
         * @brief This function applies fixed-level thresholding to a image.
         * The function is typically used to get a binary image out of a
         * grayscale image or for removing a noise, that is, filtering out
         * pixels with too small or too large values. There are several types of
         * thresholding supported by the function. They are determined by
         * type parameter. (description from opencv:threshold)
         *
         * @param threshold
         * @param max value to use when applying THRESHOLD_BINARY
         * or THRESHOLD_BINARY_INV
         * @param type threshold type (ThresholdType)
         */
        void Threshold(double threshold, double max, int type) override;

        /**
         * @brief Count the number of non-zero elements in the image
         * (SINGLE-CHANNEL).
         *
         * @return int count
         */
        int CountNonZero() override;

        /**
         * @brief Get the Size object
         *
         * @param image
         * @return Size
         */
        Size GetSize() override;

        /**
         * @brief Adds one image to this one. They need to be the same number
         * of channels.
         *
         * @param dst
         */
        void Add(Frame& imageToAdd) override;

        /**
         * @brief Encodes the image.
         *
         * @param ext File extension that defines the output format
         * @param quality Quality of the resulting image, 0-100, 100 is best
         * quality.
         * @param buffer Output buffer.
         * @return true if the image was encoded successfully
         */
        bool EncodeImage(const std::string& ext, int quality,
                         std::vector<uchar>& buffer) override;

        /**
         * @brief inverts the image.
         */
        void BitwiseNot() override;

        /**
         * @brief Modifies this image, leaving only the masked parts.
         * They should have the same size.
         *
         * @param mask
         */
        void Mask(Frame& mask) override;

       private:
        IType m_frame;
    };
}  // namespace Observer