#pragma once

#include <limits>
#include <unordered_map>
#include <vector>

#include "observer/Rect.hpp"
#include "observer/Vector.hpp"

namespace Observer {
    /**
     * @brief This class represents a blob tracked along a video. It provides
     * its bounding rect, center in a frame and velocity between two frames.
     *
     */
    class Blob {
       public:
        Blob();
        Blob(int id);

       public:
        /**
         * @brief Get blob id
         *
         * @return int
         */
        int GetId() const;

        /**
         * @brief Get the index of the first frame where it appears
         *
         * @return int
         */
        int GetFirstAppearance() const;

        /**
         * @brief Get the index of the last frame where it appears
         *
         * @return int
         */
        int GetLastAppearance() const;

        /**
         * @brief Get the velocity at the index. If not found return an empty
         * vector (isNull will be true)
         *
         * @param frameIndex
         * @return Vec
         */
        Vec GetVelocity(int frameIndex) const;

        /**
         * @brief Get the center of the blob.
         *
         * @return Point
         */
        Point GetCenter(int frameIndex) const;

        /**
         * @brief Get the bounding rect at some index. If not found then return
         * an empty rect.
         *
         * @param frameIndex
         * @return Rect
         */
        Rect GetBoundingRect(int frameIndex) const;

        /**
         * @brief Get the appearances.
         *
         * @return std::vector<int>
         */
        std::vector<int>& GetAppearances();

        /**
         * @brief Get the Average velocity magnitude
         *
         * @return double
         */
        double GetAverageMagnitude();

        double GetDistanceTraveled();

       protected:
        int id;

        std::vector<int> appearsOnFrames;

        /**
         * Blob velocities. Velocities between GetFirstAppearance and
         * GetLastAppearance are assured of its existence.
         */
        std::unordered_map<int, Vec> velocities;

        /**
         * Blob centers. Centers between GetFirstAppearance and
         * GetLastAppearance are assured of its existence.
         */
        std::unordered_map<int, Point> centers;

        /**
         * Blob bounding rects. Bounding rects between GetFirstAppearance and
         * GetLastAppearance are assured of its existence.
         */
        std::unordered_map<int, Rect> boundingRects;
    };
}  // namespace Observer