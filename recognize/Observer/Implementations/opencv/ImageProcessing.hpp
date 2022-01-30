#pragma once

#include <vector>

#include "../../src/IImageProcessing.hpp"
#include "Frame.hpp"

namespace Observer {
    class ImageProcessing : public IImageProcessing {
       public:
        void FindContours(Frame& frame,
                          std::vector<std::vector<Point>>& outContours,
                          int retrievalMode, int aproxMethod) override;
        static ImageProcessing& Get();
    };
}  // namespace Observer