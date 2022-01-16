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

        /**
         * @brief Scales each contour from the contoursSpace, the space where
         * they were found, to the target size.
         *
         * @param videoContours
         * @param contoursSpace
         */
        void ScaleContours(VideoContours& videoContours);

        void ScaleContours(FrameContours& videoContours);

        /**
         * @brief It converts all the points given by the user to the space of
         * the detected contours, so they are filtered as the user wanted no
         * matter in which space he gave them to us.
         */
        void ProcessFilters();

        bool ContourIsInsideIgnoredSet(std::vector<Point>& contour);

       private:
        ThresholdingParams params;
        ContoursFilter filters;
        FrameContextualizer<TFrame> contextBuilder;
        Size scaleTarget;

        // space in which contours will be detected (diff.size)
        Size contoursSpace {0, 0};

        // filters are processed one time.
        bool filtersProcessed {false};
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

        if (contoursSpace.width == 0) {
            contoursSpace = ImageTransformation<TFrame>::GetSize(diffFrame);
        }

        contours = this->FilterContours(contours);

        this->ScaleContours(contours);

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

        if (contoursSpace.width == 0) {
            contoursSpace = ImageTransformation<TFrame>::GetSize(diffFrames[0]);
        }

        contours = this->FilterContours(contours);

        this->ScaleContours(contours);

        return contours;
    }

    template <typename TFrame>
    FrameContours ContoursDetector<TFrame>::FilterContours(
        FrameContours& contours) {
        this->ProcessFilters();

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
                // if seems valid do the expensive checking
                if (!this->ContourIsInsideIgnoredSet(contours[i])) {
                    filtered.push_back(contours[i]);
                }
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
        if (scaleTarget == contoursSpace) return;

        double scaleX = (double)this->scaleTarget.width / contoursSpace.width;
        double scaleY = (double)this->scaleTarget.height / contoursSpace.height;

        for (auto& contour : frameContours) {
            for (auto& point : contour) {
                point.x *= scaleX;
                point.y *= scaleY;
            }
        }
    }

    template <typename TFrame>
    void ContoursDetector<TFrame>::ProcessFilters() {
        if (!filtersProcessed) {
            // scale ignored areas
            auto ref = filters.ignoredAreas.reference;
            double scaleX = (double)contoursSpace.width / ref.width;
            double scaleY = (double)contoursSpace.height / ref.height;

            for (auto& rect : filters.ignoredAreas.areas) {
                rect.x *= scaleX;
                rect.y *= scaleY;
                rect.width *= scaleX;
                rect.height *= scaleY;
            }

            // scale ignored sets
            ref = filters.ignoredSets.reference;
            scaleX = (double)contoursSpace.width / ref.width;
            scaleY = (double)contoursSpace.height / ref.height;

            for (auto& set : filters.ignoredSets.sets) {
                for (auto& point : set) {
                    point.x *= scaleX;
                    point.y *= scaleY;
                }
            }

            filtersProcessed = true;
        }
    }

    template <typename TFrame>
    bool ContoursDetector<TFrame>::ContourIsInsideIgnoredSet(
        std::vector<Point>& contour) {
        // if for some set the contour has all of its point inside it, then
        // return true, else keep searching. Hopefully is the first one.

        const double perc = filters.ignoredSets.minPercentageToIgnore / 100.0;
        for (auto& set : filters.ignoredSets.sets) {
            int contourSize = contour.size();
            int total = 0;

            for (auto& point : contour) {
                total += PointPolygonTest(set, point) > 0;
            }

            if (total >= contourSize * perc) {
                return true;
            }
        }

        return false;
    }
}  // namespace Observer