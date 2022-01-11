#pragma once

#include <opencv2/imgproc.hpp>

#include "../../src/Blob/BlobGraphics.hpp"
#include "opencv2/opencv.hpp"

namespace Observer {
    namespace {
        const cv::Scalar GetPseudoRandomColor(int val) {
            return cv::Scalar((val + 1) * 350 % 255, (val + 5) * 80 % 255,
                              (val + 20) * 200 % 255);
        }
    }  // namespace

    template <>
    struct BlobGraphics<cv::Mat> {
        static void DrawBlob(cv::Mat& frame, Blob& blob, int frameNumber,
                             double scaleX = 1, double scaleY = 1) {
            auto rect = blob.GetBoundingRect(frameNumber);
            rect.width *= scaleX;
            rect.height *= scaleY;
            rect.x *= scaleX;
            rect.y *= scaleY;

            const auto color = GetPseudoRandomColor(blob.GetId());

            cv::rectangle(frame, rect, color, 2);

            const std::string text = std::to_string(blob.GetId());

            cv::putText(frame, text, cv::Point(rect.x, rect.y),
                        cv::FONT_HERSHEY_SIMPLEX, 0.69, color, 2);
        }

        static void DrawBlobs(std::vector<cv::Mat>& frames,
                              std::vector<Blob>& blobs, double scaleX = 1,
                              double scaleY = 1) {
            for (int fi = 0; fi < frames.size(); fi++) {
                for (auto& blob : blobs) {
                    if (fi >= blob.GetFirstAppearance() &&
                        fi <= blob.GetLastAppearance()) {
                        DrawBlob(frames[fi], blob, fi, scaleX, scaleY);
                    }
                }
            }
        }

        static void DrawBlobs(cv::Mat& frame, std::vector<Blob>& blobs,
                              int positionIndex, double scaleX = 1,
                              double scaleY = 1) {
            for (auto& blob : blobs) {
                DrawBlob(frame, blob, positionIndex, scaleX, scaleY);
            }
        }
    };
}  // namespace Observer