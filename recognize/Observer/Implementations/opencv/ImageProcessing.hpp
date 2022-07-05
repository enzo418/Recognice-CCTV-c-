#pragma once

#include <vector>

#include "Frame.hpp"
#include "observer/IImageProcessing.hpp"

namespace Observer {
    class ImageProcessing : public IImageProcessing {
       public:
        void FindContours(Frame& frame,
                          std::vector<std::vector<Point>>& outContours,
                          int retrievalMode, int aproxMethod) override;
        static ImageProcessing& Get();
    };
}  // namespace Observer