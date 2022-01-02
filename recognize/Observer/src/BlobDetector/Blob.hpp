#pragma once

#include <limits>
#include <unordered_map>
#include <vector>

#include "../Rect.hpp"
#include "../Vector.hpp"

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
        int GetId();

        /**
         * @brief Get the index of the first frame where it appears
         *
         * @return int
         */
        int GetFirstAppearance();

        /**
         * @brief Get the index of the last frame where it appears
         *
         * @return int
         */
        int GetLastAppearance();

        /**
         * @brief Get the velocity at the index. If not found return an empty
         * vector (isNull will be true)
         *
         * @param frameIndex
         * @return Vec
         */
        Vec GetVelocity(int frameIndex);

        /**
         * @brief Get the center of the blob.
         *
         * @return Point
         */
        Point GetCenter(int frameIndex);

        /**
         * @brief Get the bounding rect at some index. If not found then return
         * an empty rect.
         *
         * @param frameIndex
         * @return Rect
         */
        Rect GetBoundingRect(int frameIndex);

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