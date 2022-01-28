#pragma once

#include <vector>

#include "IFrame.hpp"
#include "Point.hpp"

namespace Observer {
    // Forward declaration of the struct
    struct ImageProcessing;

    // https://docs.opencv.org/3.4/d3/dc0/group__imgproc__shape.html#ga819779b9857cc2f8601e6526a3a5bc71
    enum ContourRetrievalMode {
        CONTOUR_RETR_EXTERNAL = 0,
        CONTOUR_RETR_LIST,
        CONTOUR_RETR_CCOMP,
        CONTOUR_RETR_TREE,
        CONTOUR_RETR_FLOODFILL
    };

    // https://docs.opencv.org/3.4/d3/dc0/group__imgproc__shape.html#ga4303f45752694956374734a03c54d5ff
    enum ContourApproximationMode {
        CONTOUR_CHAIN_APPROX_NONE,
        CONTOUR_CHAIN_APPROX_SIMPLE,
        CONTOUR_CHAIN_APPROX_TC89_L1,
        CONTOUR_CHAIN_APPROX_TC89_KCOS
    };

    class IImageProcessing {
       public:
        virtual void FindContours(Frame& frame,
                                  std::vector<std::vector<Point>>& outContours,
                                  int retrievalMode, int aproxMethod) = 0;
    };
}  // namespace Observer