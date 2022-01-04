#pragma once

#include "../../ImageProcessing.hpp"
#include "../../Utils/Math.hpp"
#include "../FramesProcessor/FrameContextualizer.hpp"
#include "ContoursFilters.hpp"
#include "ContoursTypes.hpp"

namespace Observer {
    template <typename TFrame>
    class ContoursDetector {
       public:
        ContoursDetector(const ThresholdingParams& contoursParams,
                         const ContoursFilter& filterContours);

       public:
        FrameContours FindContours(TFrame& frame);
        VideoContours FindContours(std::vector<TFrame>& frames);

        int GetFrameCounter();

        /**
         * @brief Set the size to wich all the contours will be scaled before
         * beign filtered and returned.
         *
         * @param sizeToScale size to scale them
         */
        void SetScale(const Size& sizeToScale);

       protected:
        VideoContours FilterContours(VideoContours& videoContours);
        FrameContours FilterContours(FrameContours& contours);

        void ScaleContours(VideoContours& videoContours);
        void ScaleContours(FrameContours& videoContours);

       protected:
        ThresholdingParams params;
        ContoursFilter filters;
        FrameContextualizer<TFrame> contextBuilder;
        Size scaleTarget;
    };

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

        this->ScaleContours(contours);

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

        this->ScaleContours(contours);

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
                    Rect inters = rect.Intersection(j);
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

    template <typename TFrame>
    void ContoursDetector<TFrame>::SetScale(const Size& sizeToScale) {
        this->scaleTarget = sizeToScale;
    }

    template <typename TFrame>
    int ContoursDetector<TFrame>::GetFrameCounter() {
        return this->contextBuilder.frameCounter;
    }

    template <typename TFrame>
    void ContoursDetector<TFrame>::ScaleContours(VideoContours& videoContours) {
        for (auto& frameContours : videoContours) {
            this->ScaleContours(frameContours);
        }
    }

    template <typename TFrame>
    void ContoursDetector<TFrame>::ScaleContours(FrameContours& frameContours) {
        double scaleX =
            (double)this->scaleTarget.width / this->params.Resize.size.width;
        double scaleY =
            (double)this->scaleTarget.height / this->params.Resize.size.height;

        for (auto& contour : frameContours) {
            for (auto& point : contour) {
                point.x *= scaleX;
                point.y *= scaleY;
            }
        }
    }
}  // namespace Observer