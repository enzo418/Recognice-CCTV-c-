#include "Frame.hpp"

#include <cstdint>
#include <stdexcept>

namespace Observer {
    Frame::Frame() {}
    Frame::Frame(IType&& frame) { m_frame = frame; }
    Frame::Frame(const Size& size, int numberChannels) {
        m_frame = Frame::GetBlackImage(size, numberChannels);
    }

    Frame::IType& Frame::GetInternalFrame() { return m_frame; }

    Frame Frame::GetBlackImage() {
        return Frame(cv::Mat::zeros(cv::Size(m_frame.cols, m_frame.rows),
                                    m_frame.type()));
    }

    int Frame::GetNumberChannels() { return m_frame.channels(); }

    Frame::IType Frame::GetBlackImage(const Size& size, int numberChannels) {
        int type;
        switch (numberChannels) {
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
            default:
                throw std::runtime_error("channel must be <= 4");
        }

        return cv::Mat::zeros(size.height, size.width, type);
    }

    Frame Frame::Clone() { return Frame(m_frame.clone()); }

    bool Frame::IsEmpty() { return GetSize().width == 0; }

    /**
     * @brief Rotates a image
     *
     * @param angle angle, on degrees
     */
    void Frame::RotateImage(double angle) {
        cv::Point2f pc(m_frame.cols / 2., m_frame.rows / 2.);
        cv::Mat r = cv::getRotationMatrix2D(pc, angle, 1.0);
        cv::warpAffine(m_frame, m_frame, r, m_frame.size(), cv::INTER_NEAREST);
    }

    /**
     * @brief Resize the image
     *
     * @param size
     */
    void Frame::Resize(const Size& size) {
        /**
         * NOTE 1 Same resize size as original
         * ---
         * Minor performance hit. Opencv won't make a resize, nor a copy.
         * ref  modules/imgproc/src/resize.cpp#L4092
         *      modules/core/src/copy.cpp#L375
         */

        cv::Size sz(size.width, size.height);
        cv::resize(m_frame, m_frame, sz);
    }

    /**
     * @brief Resize the image
     *
     * @param scaleFactorX
     * @param scaleFactorY
     */
    void Frame::Resize(const double scaleFactorX, const double scaleFactorY) {
        cv::resize(m_frame, m_frame, cv::Size(), scaleFactorX, scaleFactorY);
    }

    /**
     * @brief Copy the image
     *
     * @param dst
     */
    void Frame::CopyTo(Frame& dst) { m_frame.copyTo(dst.GetInternalFrame()); }

    /**
     * @brief Calculates the difference between this image and `image2`.
     * This and image2 need to have the same number of channels.
     *
     * @param image2
     * @return Frame
     */
    Frame Frame::AbsoluteDifference(Frame& image2) {
        IType diff;
        cv::absdiff(m_frame, image2.GetInternalFrame(), diff);
        return Frame(std::move(diff));
    }

    /**
     * @brief Crop a image, no data is copied.
     * The destionation image pointer will be pointing to the the sub-array
     * associated with the specified roi.
     *
     * @param roi region of interest
     */
    void Frame::CropImage(const Rect& roi) {
        m_frame = m_frame(cv::Rect(roi.x, roi.y, roi.width, roi.height));
    }

    /**
     * @brief Blurs an image using a Gaussian filter
     *
     * @param radius
     */
    void Frame::GaussianBlur(int radius) {
        assert(radius > 0);
        assert(radius % 2 == 1);
        cv::GaussianBlur(m_frame, m_frame, cv::Size(radius, radius), 10);
    }

    /**
     * @brief Converts an image from one color space to another
     *
     * @param conversionType space conversion (ColorSpaceConversion)
     */
    void Frame::ToColorSpace(int conversionType) {
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
            case COLOR_RGB2BGR:
                conversionType = cv::COLOR_RGB2BGR;
                break;
            case COLOR_BGR2RGB:
                conversionType = cv::COLOR_BGR2RGB;
                break;
        }

        cv::cvtColor(m_frame, m_frame, conversionType);
    }

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
    void Frame::Threshold(double threshold, double max, int type) {
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

        cv::threshold(m_frame, m_frame, threshold, max, endtype);
    }

    /**
     * @brief Count the number of non-zero elements in the image
     * (SINGLE-CHANNEL).
     *
     * @return int count
     */
    int Frame::CountNonZero() { return cv::countNonZero(m_frame); }

    /**
     * @brief Get the Size object
     *
     * @param image
     * @return Size
     */
    Size Frame::GetSize() {
        auto sz = m_frame.size();
        return Size(sz.width, sz.height);
    }

    /**
     * @brief Adds one image to this one. They need to be the same number
     * of channels.
     *
     * @param dst
     */
    void Frame::Add(Frame& imageToAdd) {
        cv::add(m_frame, imageToAdd.GetInternalFrame(), m_frame);
    }

    /**
     * @brief Encodes the image.
     *
     * @param ext File extension that defines the output format
     * @param quality Quality of the resulting image, 0-100, 100 is best
     * quality.
     * @param buffer Output buffer.
     * @return true if the image was encoded successfully
     */
    bool Frame::EncodeImage(const std::string& ext, int quality,
                            std::vector<uchar>& buffer) {
        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, quality};
        return cv::imencode(ext, m_frame, buffer, params);
    }

    void Frame::BitwiseNot() { cv::bitwise_not(this->m_frame, this->m_frame); }

    void Frame::Mask(Frame& mask) {
        // cv::bitwise_and(this->m_frame, mask.GetInternalFrame(),
        // this->m_frame);
        this->m_frame.setTo(cv::Scalar(0, 0, 0), mask.GetInternalFrame());
    }

    uint8_t* Frame::GetData() { return m_frame.data; }
}  // namespace Observer