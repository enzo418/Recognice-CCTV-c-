#pragma once

#include <vector>

#include "../Utils/Math.hpp"
#include "Blob.hpp"
#include "BlobDetectorParams.hpp"
#include "BlobFilters.hpp"
#include "Contours/ContoursDetector.hpp"
#include "TrackingBlob.hpp"

namespace Observer {
    template <typename TFrame>
    class BlobDetector {
       public:
        BlobDetector(const BlobDetectorParams& pDetectorParams,
                     const BlobFilters& filters,
                     ContoursDetector<TFrame>& filterContours);

       public:
        std::vector<Blob> FindBlobs(TFrame& frame);
        std::vector<Blob> FindBlobs(std::vector<TFrame>& frames);

        std::vector<Blob> FindBlobs(FrameContours& frameContours);
        std::vector<Blob> FindBlobs(VideoContours& videoContours);

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

       protected:
        ContoursDetector<TFrame> contoursDetector;
        std::vector<TrackingBlob> pastTrackedBlobs;

       protected:
        BlobDetectorParams detectorParams;
        BlobFilters blobFilters;
    };

    template <typename TFrame>
    BlobDetector<TFrame>::BlobDetector(
        const BlobDetectorParams& pDetectorParams,
        const BlobFilters& pBlobFilters,
        ContoursDetector<TFrame>& pContoursDetector)
        : contoursDetector(pContoursDetector) {
        this->detectorParams = pDetectorParams;
        this->blobFilters = pBlobFilters;
    }

    template <typename TFrame>
    std::vector<Blob> BlobDetector<TFrame>::FindBlobs(TFrame& frame) {
        auto contours = this->contoursDetector.FindContours(frame);

        return this->FindBlobs(contours);
    }

    template <typename TFrame>
    std::vector<Blob> BlobDetector<TFrame>::FindBlobs(
        std::vector<TFrame>& frames) {
        auto videoContours = this->contoursDetector.FindContours(frames);

        return this->FindBlobs(videoContours);
    }

    template <typename TFrame>
    std::vector<Blob> BlobDetector<TFrame>::FindBlobs(
        FrameContours& frameContours) {
        std::vector<Blob> blobs;
        blobs = this->InternalFindBlobs(
            frameContours, this->contoursDetector.frameCounter - 1);

        this->FilterBlobs(blobs);

        return blobs;
    }

    template <typename TFrame>
    std::vector<Blob> BlobDetector<TFrame>::FindBlobs(
        VideoContours& videoContours) {
        std::vector<Blob> blobs;
        blobs = this->InternalFindBlobs(videoContours, 0, videoContours.size());

        this->FilterBlobs(blobs);

        return blobs;
    }

    template <typename TFrame>
    void BlobDetector<TFrame>::TrackBlobs(FrameContours& frameContours,
                                          int frameIndex) {
        /**
         * @brief This is the main procedure to find the blobs.
         * All this does is first if there is no blobs, add all the Findings as
         * if they were blobs. (first n frames)
         *
         * After some frames there will be blobs and some Findings, in order to
         * keep track all the blobs found previously we need to measure how
         * similar is each blob to a finding, the most similar is saved into a
         * map where each blob has cero or more findings. After doing the same
         * for all findings we join all of them for each blob and a new bonding
         * box is calculated for the resulting list of points, which is the new
         * blob.
         */
        int blobs_counter = this->pastTrackedBlobs.size();

        // current blobs = findings/objects found in the image
        auto currentBlobs = ContoursToObject(frameContours);

        bool allDead = true;

        for (auto& persistent : this->pastTrackedBlobs) {
            if (!persistent.Died()) {
                allDead = false;
                break;
            }
        }

        if (allDead || this->pastTrackedBlobs.empty()) {
            // add all the current blobs
            for (auto& blob : currentBlobs) {
                this->pastTrackedBlobs.push_back(
                    TrackingBlob(blob, blobs_counter, frameIndex,
                                 detectorParams.blob_max_life));
                blobs_counter++;
            }
        } else {
            std::unordered_map<int, std::vector<Finding>> matches;
            // 1. for every persistent blob compute the probabilty to each
            // temporal blob
            for (int i = 0; i < this->pastTrackedBlobs.size(); i++) {
                matches[i] = {};
            }

            for (auto& temporal : currentBlobs) {
                int mostSimilar = -1;
                double max_similarity = std::numeric_limits<double>::min();
                for (int i = 0; i < this->pastTrackedBlobs.size(); i++) {
                    auto& persistent = this->pastTrackedBlobs[i];
                    double similarity;
                    if (!persistent.Died()) {
                        similarity = persistent.Similarity(
                            temporal, detectorParams.distance_thresh);
                        if (similarity > max_similarity &&
                            similarity > detectorParams.similarity_threshold) {
                            max_similarity = similarity;
                            mostSimilar = i;
                        }
                    }
                }

                if (mostSimilar != -1) {
                    matches[mostSimilar].push_back(temporal);
                } else {
                    this->pastTrackedBlobs.push_back(
                        TrackingBlob(temporal, blobs_counter, frameIndex,
                                     detectorParams.blob_max_life));
                    blobs_counter++;
                }
            }

            // 2. for each temporal blob, assign it to the one with the most
            // probability
            for (auto const& [key, blobs] : matches) {
                if (!blobs.empty()) {
                    auto sz = 0;

                    for (auto blob : blobs) {
                        sz += blob.GetPoints().size();
                    }

                    // 3. Get a temporal blob than includes all of them
                    std::vector<Point> points(sz);

                    int lastp = 0;
                    for (auto blob : blobs) {
                        auto bp = std::move(blob.TakePoints());
                        int n = bp.size();
                        std::swap_ranges(points.begin() + lastp,
                                         points.begin() + lastp + bp.size(),
                                         bp.begin());
                        lastp += n;
                    }

                    Rect rect = BoundingRect(points);

                    // 4. set the persistent blob to this one
                    this->pastTrackedBlobs[key].Become(rect, frameIndex);

                    // 5. reset its life left
                    this->pastTrackedBlobs[key].AddLife(
                        detectorParams.blob_max_life);
                } else {
                    this->pastTrackedBlobs[key].ReduceLife(frameIndex);

                    // if it died, do nothing. It's a logic deletion.
                    //
                    // if (this->pastTrackedBlobs[key].Died()) {
                    //     this->pastTrackedBlobs.erase(this->pastTrackedBlobs.begin()
                    //     + key);
                    // }
                }
            }
        }
    }

    template <typename TFrame>
    std::vector<Finding> BlobDetector<TFrame>::ContoursToObject(
        FrameContours& contours) {
        std::vector<Finding> currentBlobs;
        for (auto& contour : contours) {
            for (auto& point : contour) {
                double shortest_distance = std::numeric_limits<double>::max();
                Finding* closest = nullptr;

                for (auto& blob : currentBlobs) {
                    double distance = blob.GetShortestDistance(point);
                    if (distance < this->detectorParams.distance_thresh &&
                        distance < shortest_distance) {
                        closest = &blob;
                        shortest_distance = distance;
                    }
                }

                if (closest != nullptr) {
                    closest->AddPoint(point);
                } else {
                    currentBlobs.push_back(Finding(point));
                }
            }
        }

        return currentBlobs;
    }

    template <typename TFrame>
    void BlobDetector<TFrame>::FilterBlobs(std::vector<Blob>& blobs) {
        std::vector<Blob> filtered;
        for (int i = 0; i < blobs.size(); i++) {
            auto& blob = blobs[i];
            if (blob.GetLastAppearance() - blob.GetFirstAppearance() >=
                this->blobFilters.MinimumOccurrences) {
                filtered.push_back(std::move(blob));
            }
        }

        blobs = filtered;
    }

    template <typename TFrame>
    std::vector<Blob> BlobDetector<TFrame>::InternalFindBlobs(
        VideoContours& contours, int frameIndexStart, int frameIndexEnd) {
        for (int frameIndex = frameIndexStart; frameIndex < frameIndexEnd;
             frameIndex++) {
            TrackBlobs(contours[frameIndex], frameIndex);
        }

        return TrackingBlobToBlob(this->pastTrackedBlobs);
    }

    template <typename TFrame>
    std::vector<Blob> BlobDetector<TFrame>::InternalFindBlobs(
        FrameContours& contours, int frameIndex) {
        TrackBlobs(contours, frameIndex);

        return TrackingBlobToBlob(this->pastTrackedBlobs);
    }
}  // namespace Observer