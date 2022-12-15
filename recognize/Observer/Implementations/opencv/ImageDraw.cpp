#include "ImageDraw.hpp"

namespace Observer {
    void ImageDraw::FillConvexPoly(Frame& image,
                                   const std::vector<Point>& points,
                                   const ScalarVector& color) {
        cv::Point cvPoints[points.size()];
        for (int i = 0; i < points.size(); i++) {
            cvPoints[i] = points[i];
        }

        cv::fillConvexPoly(image.GetInternalFrame(), cvPoints, points.size(),
                           cv::Scalar(color.b, color.g, color.r));
    }

    ImageDraw& ImageDraw::Get() {
        static ImageDraw d;
        return d;
    }
}  // namespace Observer