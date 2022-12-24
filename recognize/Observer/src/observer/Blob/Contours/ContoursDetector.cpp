#include "ContoursDetector.hpp"

namespace Observer {
    ContoursDetector::ContoursDetector(const ThresholdParams& thresholdParams,
                                       const ContoursFilter& filterContours)
        : contextBuilder(thresholdParams),
          params(thresholdParams),
          filters(filterContours) {}

    FrameContours ContoursDetector::FindContours(Frame& frame) {
        // 1. Complete diff frames
        auto diffFrame = this->contextBuilder.GenerateDiffFrame(frame);

        FrameContours contours;

        ImageProcessing::Get().FindContours(
            diffFrame, contours, ContourRetrievalMode::CONTOUR_RETR_LIST,
            ContourApproximationMode::CONTOUR_CHAIN_APPROX_SIMPLE);

        if (contoursSpace.width == 0) {
            contoursSpace = diffFrame.GetSize();
        }

        contours = this->FilterContours(contours);

        this->ScaleContours(contours);

        return contours;
    }

    VideoContours ContoursDetector::FindContoursFromDiffFrames(
        const std::vector<Frame>& pDiffFrames) {
        std::vector<Frame> diffFrames = std::move(pDiffFrames);

        VideoContours contours(diffFrames.size());

        for (int i = 0; i < diffFrames.size(); i++) {
            ImageProcessing::Get().FindContours(
                diffFrames[i], contours[i],
                ContourRetrievalMode::CONTOUR_RETR_LIST,
                ContourApproximationMode::CONTOUR_CHAIN_APPROX_SIMPLE);
        }

        if (contoursSpace.width == 0) {
            contoursSpace = diffFrames[0].GetSize();
        }

        contours = this->FilterContours(contours);

        this->ScaleContours(contours);

        return contours;
    }

    VideoContours ContoursDetector::FindContours(std::vector<Frame>& frames) {
        return FindContoursFromDiffFrames(
            this->contextBuilder.GenerateDiffFrames(frames));
    }

    FrameContours ContoursDetector::FilterContours(FrameContours& contours) {
        this->ProcessFilters();

        FrameContours filtered;

        double avgArea = 0;
        int contours_size = 0;
        std::vector<double> contours_areas(contours.size());
        std::vector<bool> contour_overlap(contours.size(), false);

        // Get the area of each contour and calculate the average
        for (int i = 0; i < contours.size(); i++) {
            Rect rect = BoundingRect(contours[i]);
            double area = rect.area();
            avgArea += area;
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

        avgArea /= contours.size();

        // Delete all the contours that doesn't comply with the filters
        auto sz = contours.size();
        for (int i = 0; i < sz; i++) {
            const bool areaFilter = (contours_areas[i] >= avgArea ||
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

    VideoContours ContoursDetector::FilterContours(
        VideoContours& videoContours) {
        VideoContours filtered(videoContours.size());

        for (int j = 0; j < videoContours.size(); j++) {
            filtered[j] = this->FilterContours(videoContours[j]);
        }

        return filtered;
    }

    void ContoursDetector::SetScale(const Size& sizeToScale) {
        this->scaleTarget = sizeToScale;
    }

    int ContoursDetector::GetFrameCounter() {
        return this->contextBuilder.frameCounter;
    }

    void ContoursDetector::ScaleContours(VideoContours& videoContours) {
        for (auto& frameContours : videoContours) {
            this->ScaleContours(frameContours);
        }
    }

    void ContoursDetector::ScaleContours(FrameContours& frameContours) {
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

    void ContoursDetector::ProcessFilters() {
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

    bool ContoursDetector::ContourIsInsideIgnoredSet(
        std::vector<Point>& contour) {
        // if for some set the contour has all of its point inside it, then
        // return true, else keep searching. Hopefully is the first one.

        const double percentage =
            filters.ignoredSets.minPercentageToIgnore / 100.0;
        for (auto& set : filters.ignoredSets.sets) {
            int contourSize = contour.size();
            int total = 0;

            for (auto& point : contour) {
                total += PointPolygonTest(set, point) > 0;
            }

            if (total >= contourSize * percentage) {
                return true;
            }
        }

        return false;
    }
}  // namespace Observer