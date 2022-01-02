
#include "ContoursDetector.hpp"

namespace Observer {
    template <typename TFrame>
    ContoursDetector<TFrame>::ContoursDetector(
        const ThresholdingParams& contoursParams,
        const ContoursFilter& filterContours)
        : contextBuilder(contoursParams) {
        this->filters = filterContours;
        this->params = contoursParams;
    }

    template <typename TFrame>
    FrameContours ContoursDetector<TFrame>::FindContours(TFrame& frame) {
        // 1. Complete diff frames
        auto diffFrame = this->contextBuilder.GenerateDiffFrame(frame);

        FrameContours contours;

        ImageProcessing<TFrame>::FindContours(
            diffFrame, contours, ContourRetrievalMode::CONTOUR_RETR_LIST,
            ContourApproximationMode::CONTOUR_CHAIN_APPROX_SIMPLE);

        contours = this->FilterContours(contours);

        return contours;
    }

    template <typename TFrame>
    VideoContours ContoursDetector<TFrame>::FindContours(
        std::vector<TFrame>& frames) {
        VideoContours contours(frames.size());

        std::vector<TFrame> diffFrames =
            this->contextBuilder.GenerateDiffFrames(frames);

        for (int i = 0; i < diffFrames.size(); i++) {
            ImageProcessing<TFrame>::FindContours(
                diffFrames[i], contours[i],
                ContourRetrievalMode::CONTOUR_RETR_LIST,
                ContourApproximationMode::CONTOUR_CHAIN_APPROX_SIMPLE);
        }

        contours = this->FilterContours(contours);

        return contours;
    }

    template <typename TFrame>
    FrameContours ContoursDetector<TFrame>::FilterContours(
        FrameContours& contours) {
        FrameContours filtered;

        double avrgArea = 0;
        int contorus_size = 0;
        std::vector<double> contours_areas(contours.size());
        std::vector<bool> contour_overlap(contours.size(), false);

        // Get the area of each contour and calcule the average
        for (int i = 0; i < contours.size(); i++) {
            Rect rect = BoundingRect(contours[i]);
            double area = rect.area();
            avrgArea += area;
            contours_areas[i] = area;

            // check for intersection with ignored areas
            if (!filters.ignoredAreas.areas.empty()) {
                for (auto&& j : filters.ignoredAreas.areas) {
                    Rect inters = rect & j;
                    if (inters.area() >=
                        rect.area() *
                            filters.ignoredAreas.minAreaPercentageToIgnore) {
                        contour_overlap[i] = true;
                    }
                }
            }
        }

        avrgArea /= contours.size();

        // Delete all the contours that doesn't comply with the filters
        auto sz = contours.size();
        for (int i = 0; i < sz; i++) {
            const bool areaFilter = (contours_areas[i] >= avrgArea ||
                                     !filters.FilterByAverageArea) &&
                                    contours_areas[i] >= filters.MinimumArea;

            const bool overlapFilter = !contour_overlap[i];

            const bool didPassFilter = areaFilter && overlapFilter;

            if (didPassFilter) {
                filtered.push_back(contours[i]);
            }
        }
        return filtered;
    }

    template <typename TFrame>
    VideoContours ContoursDetector<TFrame>::FilterContours(
        VideoContours& videoContours) {
        VideoContours filtered(videoContours.size());

        for (int j = 0; j < videoContours.size(); j++) {
            filtered[j] = this->FilterContours(videoContours[j]);
        }

        return filtered;
    }
}  // namespace Observer