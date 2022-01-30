#pragma once

#include <vector>

#include "../../IFrame.hpp"
#include "../../Log/log.hpp"
#include "../../Utils/Math.hpp"
#include "../Contours/ContoursDetector.hpp"
#include "Blob.hpp"
#include "BlobDetectorParams.hpp"
#include "BlobFilters.hpp"
#include "TrackingBlob.hpp"

namespace Observer {
    class BlobDetector {
       public:
        BlobDetector(const BlobDetectorParams& pDetectorParams,
                     const BlobFilters& filters,
                     ContoursDetector& filterContours);

       public:
        std::vector<Blob> FindBlobs(Frame& frame);
        std::vector<Blob> FindBlobs(std::vector<Frame>& frames);

        std::vector<Blob> FindBlobs(FrameContours& frameContours);
        std::vector<Blob> FindBlobs(VideoContours& videoContours);

       public:
        /**
         * @brief Set the size to wich all the blobs will be scaled before beign
         * returned.
         *
         * @param sizeToScale size to scale them
         */
        void SetScale(const Size& sizeToScale);

       protected:
        /**
         * @brief Handles all the calls to track blob and conversion of blobs.
         *
         * @param contours
         * @param frameIndexStart
         * @param frameIndexEnd
         * @return std::vector<Blob>
         */
        std::vector<Blob> InternalFindBlobs(VideoContours& contours,
                                            int frameIndexStart,
                                            int frameIndexEnd);

        /**
         * @brief Handles the call to track blob and conversion of blobs.
         *
         * @param contours
         * @param frameIndex
         * @return std::vector<Blob>
         */
        std::vector<Blob> InternalFindBlobs(FrameContours& contours,
                                            int frameIndex);

        /**
         * @brief It groups all the objects found on the contours and calculates
         * how likely it is the same as some know blob that we have recorded
         * before. If it's the same then it joins it with others that also are
         * similar and then saves it internally as a blob.
         *
         * @param frameContours
         * @param frameIndex
         */
        void TrackBlobs(FrameContours& frameContours, int frameIndex);

        /**
         * @brief It groups all contours that are within some maximum distance,
         * all the rest is considered a physical object.
         *
         * @param contours
         * @return std::vector<Finding>
         */
        std::vector<Finding> ContoursToObject(FrameContours& contours);

        /**
         * @brief Filters all the blobs based on the blobFilters member.
         *
         * @param blobs
         */
        void FilterBlobs(std::vector<Blob>& blobs);

       private:
        std::vector<Blob> TrackingBlobToBlob(std::vector<TrackingBlob>& blobs);

       protected:
        ContoursDetector contoursDetector;
        std::vector<TrackingBlob> pastTrackedBlobs;

       protected:
        BlobDetectorParams detectorParams;
        BlobFilters blobFilters;

        // obtained from the current frame size, used to filter blobs
        double unitLength;
    };
}  // namespace Observer