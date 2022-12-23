#include "ImageDraw.hpp"

namespace Observer {
    void ImageDraw::FillConvexPoly(Frame& image,
                                   const std::vector<Point>& points,
                                   const ScalarVector& color) {
        cv::Point cvPoints[points.size()];
        for (int i = 0; i < points.size(); i++) {
            cvPoints[i] = points[i];
        }

        cv::fillConvexPoly(image.GetInternalFrame(), cvPoints,
                           (int)points.size(),
                           cv::Scalar(color.b, color.g, color.r));
    }

    void ImageDraw::DrawContours(
        std::vector<Frame>& frames,
        const std::vector<std::vector<std::vector<Point>>>& videoContours,
        const ScalarVector& color) {
        const size_t framesSize = frames.size();
        for (size_t i = 0; i < framesSize; i++) {
            const std::vector<std::vector<Point>>& frameContours =
                videoContours[i];

            std::vector<std::vector<cv::Point>> cvFrameContours(
                frameContours.size());

            for (int c = 0; c < frameContours.size(); c++) {
                cvFrameContours[c].assign(frameContours[c].begin(),
                                          frameContours[c].end());
            }

            for (size_t j = 0; j < cvFrameContours.size(); j++) {
                cv::drawContours(frames[i].GetInternalFrame(), cvFrameContours,
                                 (int)j, cv::Scalar(color.b, color.g, color.r),
                                 2, cv::LINE_8);
            }
        }
    }

    ImageDraw& ImageDraw::Get() {
        static ImageDraw d;
        return d;
    }
}  // namespace Observer