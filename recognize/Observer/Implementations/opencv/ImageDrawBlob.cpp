#include "ImageDrawBlob.hpp"

namespace Observer {

    namespace {
        const cv::Scalar GetPseudoRandomColor(int val) {
            return cv::Scalar((val + 1) * 350 % 255, (val + 5) * 80 % 255,
                              (val + 20) * 200 % 255);
        }
    }  // namespace

    void ImageDrawBlob::DrawBlob(Frame& frame, Blob& blob, int frameNumber,
                                 double scaleX, double scaleY) {
        auto rect = blob.GetBoundingRect(frameNumber);
        rect.width *= scaleX;
        rect.height *= scaleY;
        rect.x *= scaleX;
        rect.y *= scaleY;

        const auto color = GetPseudoRandomColor(blob.GetId());

        cv::rectangle(frame.GetInternalFrame(), rect, color, 2);

        const std::string text = std::to_string(blob.GetId());

        cv::putText(frame.GetInternalFrame(), text, cv::Point(rect.x, rect.y),
                    cv::FONT_HERSHEY_SIMPLEX, 0.69, color, 2);
    }

    void ImageDrawBlob::DrawBlobs(Frame& frame, std::vector<Blob>& blobs,
                                  int positionIndex, double scaleX,
                                  double scaleY) {
        for (auto& blob : blobs) {
            DrawBlob(frame, blob, positionIndex, scaleX, scaleY);
        }
    }

    /**
     * @brief Draws all the blobs that appear in each frame.
     *
     * @param frames frames to draw on
     * @param blobs blobs
     * @param scaleX scales x axis
     * @param scaleY scales y axis
     */
    void ImageDrawBlob::DrawBlobs(std::vector<Frame>& frames,
                                  std::vector<Blob>& blobs, double scaleX,
                                  double scaleY) {
        for (size_t fi = 0; fi < frames.size(); fi++) {
            for (auto& blob : blobs) {
                if ((int)fi >= blob.GetFirstAppearance() &&
                    (int)fi <= blob.GetLastAppearance()) {
                    DrawBlob(frames[fi], blob, fi, scaleX, scaleY);
                }
            }
        }
    }

    /**
     * @brief Get the instance
     *
     * @return ImageDrawBlob&
     */
    ImageDrawBlob& ImageDrawBlob::Get() {
        static ImageDrawBlob inst;
        return inst;
    }
}  // namespace Observer