#pragma once

namespace Observer {

    struct ScalarVector {
        double r {0};
        double g {0};
        double b {0};
        // double a {1};

        // BGR A (as opencv)
        ScalarVector(double pB, double pG, double pR) : b(pB), g(pG), r(pR) {}

        // color for single channel image, 1 = white and 0 = black
        static ScalarVector Black() { return ScalarVector(0, 0, 0); }

        // color for single channel image
        static ScalarVector White() { return ScalarVector(255, 255, 255); }
    };
}  // namespace Observer