#include "ImageDrawBlob.hpp"

#include <opencv2/imgproc.hpp>

#include "observer/Domain/Classification/BlobClassification.hpp"

namespace Observer {

    namespace {
        const cv::Scalar GetPseudoRandomColor(int val) {
            return cv::Scalar((val + 1) * 350 % 255, (val + 5) * 80 % 255,
                              (val + 20) * 200 % 255);
        }
    }  // namespace

    void ImageDrawBlob::DrawBlob(Frame& frame, Blob& blob,
                                 const BlobClassification* classification,
                                 int frameNumber, double scaleX,
                                 double scaleY) {
        static const int padding = 6;

        // if (classification == nullptr) {
        //     return;
        // }

        auto rect = blob.GetBoundingRect(frameNumber);
        rect.width *= scaleX;
        rect.height *= scaleY;
        rect.x *= scaleX;
        rect.y *= scaleY;

        if (rect.x < 0 || rect.y < 0 || rect.area() == 0) {
            return;
        }

        const auto color = GetPseudoRandomColor(blob.GetId());

        cv::rectangle(frame.GetInternalFrame(), rect, color, 2);

        const std::string text =
            classification == nullptr
                ? std::to_string(blob.GetId())
                : classification->label + " " +
                      std::to_string((int)(classification->confidence * 100)) +
                      "% (" + std::to_string((int)(classification->IoU * 100)) +
                      " % IoU)";

        int bl;
        cv::Size txtSize =
            cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.69, 2, &bl);

        cv::Point txtPos =
            rect.y < txtSize.height + padding
                ? cv::Point(rect.x, rect.y + rect.height + txtSize.height)
                : cv::Point(rect.x, rect.y - padding);

        cv::putText(frame.GetInternalFrame(), text, txtPos,
                    cv::FONT_HERSHEY_SIMPLEX, 0.69, color, 2, cv::LINE_AA);
    }

    void ImageDrawBlob::DrawBlobs(Frame& frame, std::vector<Blob>& blobs,
                                  const BlobClassifications& classifications,
                                  int positionIndex, double scaleX,
                                  double scaleY) {
        for (auto& blob : blobs) {
            const BlobClassification* classification =
                classifications.contains(blob.GetId())
                    ? &classifications.at(blob.GetId())
                    : nullptr;
            DrawBlob(frame, blob, classification, positionIndex, scaleX,
                     scaleY);
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
                                  std::vector<Blob>& blobs,
                                  const BlobClassifications& classifications,
                                  double scaleX, double scaleY) {
        for (size_t fi = 0; fi < frames.size(); fi++) {
            for (auto& blob : blobs) {
                if ((int)fi >= blob.GetFirstAppearance() &&
                    (int)fi <= blob.GetLastAppearance()) {
                    const BlobClassification* classification =
                        classifications.contains(blob.GetId())
                            ? &classifications.at(blob.GetId())
                            : nullptr;
                    DrawBlob(frames[fi], blob, classification, fi, scaleX,
                             scaleY);
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