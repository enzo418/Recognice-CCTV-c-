#include "ContoursDetector.hpp"

namespace Observer {
    ContoursDetector::ContoursDetector(const ThresholdParams& thresholdParams,
                                       const ContoursFilter& filterContours)
        : params(thresholdParams),
          filters(filterContours),
          contextBuilder(thresholdParams) {}

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
        std::vector<Frame>& diffFrames) {
        VideoContours contours(diffFrames.size());

        for (size_t i = 0; i < diffFrames.size(); i++) {
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

    VideoContours ContoursDetector::FindContoursFromDiffFrames(
        std::vector<Frame>&& pDiffFrames) {
        std::vector<Frame> diffFrames = std::move(pDiffFrames);

        VideoContours contours(diffFrames.size());

        for (size_t i = 0; i < diffFrames.size(); i++) {
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
        std::vector<double> contours_areas(contours.size());
        std::vector<bool> contour_overlap(contours.size(), false);

        // Get the area of each contour and calculate the average
        for (size_t i = 0; i < contours.size(); i++) {
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
        for (size_t i = 0; i < sz; i++) {
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

        for (size_t j = 0; j < videoContours.size(); j++) {
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

            // Draw ignored sets on frame
            this->ignoredSetsFrame = Frame(contoursSpace, 1);
            for (auto& set : filters.ignoredSets.sets) {
                ImageDraw::Get().FillAnyPoly(this->ignoredSetsFrame, set,
                                             ScalarVector::White());
            }

            filtersProcessed = true;
        }
    }

    bool ContoursDetector::ContourIsInsideIgnoredSet(
        std::vector<Point>& contour) {
        // if for some set the contour has all of its point inside it, then
        // return true, else keep searching.

        // On how we calculate the area of intersection:
        // ---
        // To check if it's inside the ignored sets first we draw all the
        // ignored sets in a frame as white polygons, at "ProcessFilters".
        // Then we draw the contour as black over those polygons, if it's inside
        // some ignored sets it will remove white pixels from the image and the
        // difference between those images will be the area of the contour that
        // intersects with those sets, otherwise it will not affect the sets and
        // the difference will be 0.

        Frame diff;
        Frame polyWithMask;

        const double percentage =
            filters.ignoredSets.minPercentageToIgnore / 100.0;

        polyWithMask = this->ignoredSetsFrame.Clone();

        ImageDraw::Get().FillConvexPoly(polyWithMask, contour,
                                        ScalarVector::Black());

        diff = this->ignoredSetsFrame.AbsoluteDifference(polyWithMask);

        return diff.CountNonZero() >= PolygonArea(contour) * percentage;
    }
}  // namespace Observer