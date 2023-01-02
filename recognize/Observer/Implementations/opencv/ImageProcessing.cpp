#include "ImageProcessing.hpp"

namespace Observer {
    namespace {
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
    }  // namespace

    void ImageProcessing::FindContours(
        Frame& pFrame, std::vector<std::vector<Point>>& pOutContours,
        int pRetrievalMode, int pAproxMethod) {
        int retrievalmode = ParseContourRetrievalMode(pRetrievalMode);
        int aproxMethod = ParseContourAproxMode(pAproxMethod);

        std::vector<std::vector<cv::Point>> contours;

        cv::findContours(pFrame.GetInternalFrame(), contours, retrievalmode,
                         aproxMethod);

        // convert to Observer Point
        pOutContours.resize(contours.size());

        for (size_t i = 0; i < contours.size(); i++) {
            pOutContours[i].reserve(contours[i].size());

            pOutContours[i].insert(pOutContours[i].end(), contours[i].begin(),
                                   contours[i].end());
        }
    }

    ImageProcessing& ImageProcessing::Get() {
        static ImageProcessing instance;
        return instance;
    }
}  // namespace Observer