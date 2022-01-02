#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

#include "../../src/ImageProcessing.hpp"

namespace Observer {

    static int ParseContourRetrievalMode(int pRetrievalMode) {
        int retrievalmode;
        switch (pRetrievalMode) {
            case CONTOUR_RETR_EXTERNAL:
                retrievalmode = cv::RETR_EXTERNAL;
                break;
            case CONTOUR_RETR_LIST:
                retrievalmode = cv::RETR_LIST;
                break;
            case CONTOUR_RETR_CCOMP:
                retrievalmode = cv::RETR_CCOMP;
                break;
            case CONTOUR_RETR_TREE:
                retrievalmode = cv::RETR_TREE;
                break;
            case CONTOUR_RETR_FLOODFILL:
                retrievalmode = cv::RETR_FLOODFILL;
                break;
        }

        return retrievalmode;
    }

    static int ParseContourAproxMode(int pAproxMethod) {
        int aproxMethod;
        switch (pAproxMethod) {
            case CONTOUR_CHAIN_APPROX_NONE:
                aproxMethod = cv::CHAIN_APPROX_NONE;
                break;
            case CONTOUR_CHAIN_APPROX_SIMPLE:
                aproxMethod = cv::CHAIN_APPROX_SIMPLE;
                break;
            case CONTOUR_CHAIN_APPROX_TC89_L1:
                aproxMethod = cv::CHAIN_APPROX_TC89_L1;
                break;
            case CONTOUR_CHAIN_APPROX_TC89_KCOS:
                aproxMethod = cv::CHAIN_APPROX_TC89_KCOS;
                break;
        }

        return aproxMethod;
    }

    template <typename TFrame>
    static void FindContours(TFrame& pFrame, std::vector<Point>& pOutContours,
                             int pRetrievalMode, int pAproxMethod) {
        int retrievalmode = ParseContourRetrievalMode(pRetrievalMode);
        int aproxMethod = ParseContourAproxMode(pAproxMethod);

        // no explicit conversion is needed
        cv::findContours(pFrame, pOutContours, retrievalmode, aproxMethod);
    }
}  // namespace Observer