#pragma once

#include "../FramesProcessor/FrameContextualizer.hpp"
#include "ContoursFilters.hpp"
#include "ContoursTypes.hpp"
#include "observer/IFrame.hpp"
#include "observer/Implementation.hpp"
#include "observer/Utils/Math.hpp"

namespace Observer {
    class ContoursDetector {
       public:
        ContoursDetector(const ThresholdParams& thresholdingParams,
                         const ContoursFilter& filterContours);

       public:
        FrameContours FindContours(Frame& frame);
        VideoContours FindContours(std::vector<Frame>& frames);

        /**
         * @brief Same as above but expects frames with 1 channel
         *
         * @param frames 1 channel frame list
         * @return VideoContours
         */
        VideoContours FindContoursFromDiffFrames(
            std::vector<Frame>& diffFrames);

        VideoContours FindContoursFromDiffFrames(
            std::vector<Frame>&& diffFrames);

        int GetFrameCounter();

        /**
         * @brief Set the size to which all the contours will be scaled before
         * being filtered and returned.
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

       public:
        /**
         * @brief Initialize external (e.g. opencv) components
         * This function is ONLY defined in a unit inside the implementation
         * folder.
         */
        void InitializeComponentsImplementation();

       public:
        ThresholdParams params;
        ContoursFilter filters;
        FrameContextualizer contextBuilder;
        Size scaleTarget;

        // space in which contours will be detected (diff.size)
        Size contoursSpace {0, 0};

        // filters are processed one time.
        bool filtersProcessed {false};

        // has the ignored sets drawn, more on ContourIsInsideIgnored
        Frame ignoredSetsFrame;
    };
}  // namespace Observer