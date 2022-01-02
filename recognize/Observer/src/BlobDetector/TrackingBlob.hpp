#pragma once

#include <opencv2/core/types.hpp>
#include <unordered_map>

#include "../Utils/Math.hpp"
#include "../Vector.hpp"
#include "Blob.hpp"
#include "Finding.hpp"

#define LERP true

namespace Observer {
    class TrackingBlob : public Blob {
       public:
        TrackingBlob(Finding other, int id, int frameIndex,
                     int initialLife = 50);

        /**
         * @brief Updates the bounding rect of the blob to `rect`
         *
         * @param rect
         * @param frameIndex
         */
        void Become(Rect rect, int frameIndex);

       public:
        /**
         * @brief Calculates the probability that the finding is part of the
         * blob.
         *
         * @param other
         * @param max_distance
         * @return double
         */
        double Similarity(Finding& other, int max_distance);

        /**
         * @brief Reduce the life of the blob, in the case it's missing.
         *
         * @param frameIndex
         */
        void ReduceLife(int frameIndex);

        /**
         * @brief Add life to the blob
         *
         * @param extraLife
         */
        void AddLife(int extraLife);

        /**
         * @brief Get the life left of the blob
         *
         * @return int
         */
        int GetLifeLeft();

        /**
         * @brief Wether the blob run out of time or not.
         *
         * @param frameIndex
         * @return true
         * @return false
         */
        bool Died();

        /**
         * @brief Computes the distance between this blob and a finding.
         *
         * @param finding
         * @return double
         */
        double DistanceTo(Finding& finding);

       public:
        /**
         * @brief Converts from a tracking blob to a Blob.
         *
         * @return Blob
         */
        Blob ToBlob();

       protected:
        /**
         * @brief Set the center of the tracking blob based on a rect.
         *
         * @param rect
         */
        void SetCenter(Rect& rect);

       private:
        int lifeLeft;
        int maxLife;

       private:
        Rect boundingRect;
        Point center;
    };

}  // namespace Observer