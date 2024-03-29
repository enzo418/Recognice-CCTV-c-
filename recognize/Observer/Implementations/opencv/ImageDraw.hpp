#pragma once

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "Frame.hpp"
#include "observer/IImageDraw.hpp"

namespace Observer {
    /**
     * @brief Singleton implementation of image draw blob
     *
     */
    class ImageDraw final : public IImageDraw {
       public:
        void FillConvexPoly(Frame& image, const std::vector<Point>& points,
                            const ScalarVector& color) override;

        void FillAnyPoly(Frame& image, const std::vector<Point>& points,
                         const ScalarVector& color) override;

        void DrawContours(
            std::vector<Frame>& frames,
            const std::vector<std::vector<std::vector<Point>>>& videoContours,
            const ScalarVector& color) override;

        static ImageDraw& Get();
    };
}  // namespace Observer