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

       public:
        long frameCounter {0};

       protected:
        VideoContours FilterContours(VideoContours& videoContours);
        FrameContours FilterContours(FrameContours& contours);

       protected:
        ThresholdingParams params;
        ContoursFilter filters;
        FrameContextualizer<TFrame> contextBuilder;
    };
}  // namespace Observer