#include "TrackingBlob.hpp"

namespace Observer {
    TrackingBlob::TrackingBlob(Finding other, int id, int frameIndex, int life)
        : Blob(id) {
        this->id = id;
        this->lifeLeft = life;
        this->maxLife = life;

        this->boundingRect = other.GetBoundingRect();
        this->boundingRects[frameIndex] = this->boundingRect;

        this->SetCenter(this->boundingRect);
        this->centers[frameIndex] = this->center;

        // to avoid not having a velocity in the first apparence, make it null
        this->velocities[frameIndex] = Vec(this->center, this->center);

        this->appearsOnFrames.push_back(frameIndex);
    }

    bool TrackingBlob::Died() { return lifeLeft <= 0; }

    void TrackingBlob::ReduceLife(int frameIndex) {
        this->lifeLeft--;

        if (this->Died()) {
            this->appearsOnFrames.push_back(frameIndex);
        }
    }

    void TrackingBlob::AddLife(int extraLife) {
        this->lifeLeft =
            this->lifeLeft <= 0 ? extraLife : this->lifeLeft + extraLife;
        this->lifeLeft =
            this->lifeLeft > this->maxLife ? this->maxLife : this->lifeLeft;
    }

    int TrackingBlob::GetLifeLeft() { return lifeLeft; }

    void TrackingBlob::Become(Rect rect, int frameIndex) {
        auto oldCenterBR = this->center;
        auto oldBR = this->boundingRect;

        this->boundingRect = rect;
        this->SetCenter(this->boundingRect);

        auto newCenterBR = this->center;

        int lastFrameDetected = this->GetLastAppearance();
        double notSeenFor = frameIndex - lastFrameDetected;

        // centers empty -> first call to become
        if (!LERP || notSeenFor == 1) {
            this->centers[frameIndex] = newCenterBR;

            this->velocities[frameIndex] = Vec(oldCenterBR, newCenterBR);
        } else if (LERP) {
            /**
             * Build all the velocities. If the index of the last center is
             * exactly the previous one to this frame index, then just add the
             * velocity from the last center to the new one. Otherwise, do a
             * linear interpolation where the distance of each segment is equal
             * to d/t, and t = thisIndex - lastIndex and d =
             * distance(thisCenter, lastCenter).
             * Similarly for bounding rects and centers.
             */

            if (oldCenterBR != newCenterBR) {
                // Build missing centers
                auto missingCenters =
                    Interpolate(oldCenterBR, newCenterBR, notSeenFor);
                for (int i = 0; i < missingCenters.size(); i++) {
                    const int fi = lastFrameDetected + i + 1;
                    this->centers[fi] = missingCenters[i];
                }

                this->centers[frameIndex] = newCenterBR;
            } else {
                for (int i = 0; i < notSeenFor; i++) {
                    const int fi = lastFrameDetected + i + 1;
                    this->centers[fi] = newCenterBR;
                }
            }

            // Build missing velocities, based on the interpolated centers
            for (int i = 0; i < notSeenFor; i++) {
                const int fi = lastFrameDetected + i;
                this->velocities[fi] =
                    Vec(this->centers[fi], this->centers[fi + 1]);
            }

            // Build missing bounding rects, interpolate the last bounding rect
            // into this one
            auto interpolatedTL =
                Interpolate(oldBR.tl(), this->boundingRect.tl(), notSeenFor);
            auto interpolatedBR =
                Interpolate(oldBR.br(), this->boundingRect.br(), notSeenFor);
            for (int i = 0; i < interpolatedTL.size(); i++) {
                const int fi = lastFrameDetected + i + 1;
                auto rect = Rect(interpolatedTL[i], interpolatedBR[i]);
                this->boundingRects[fi] = rect;
            }
        }

        this->boundingRects[frameIndex] = this->boundingRect;
        appearsOnFrames.push_back(frameIndex);
    }

    double TrackingBlob::DistanceTo(Finding& finding) {
        double distance = 0;
        // 1. Check if they overlap
        auto r1 = this->boundingRect;
        auto r2 = finding.GetBoundingRect();
        auto inter = r1.Intersection(r2);

        if (inter.empty()) {
            // 1 -> They don't overlap
            // 2. Check the distance between tl and br from both
            distance = std::min(std::min(r1.tl().DistanceTo(r2.tl()),
                                         r1.tl().DistanceTo(r2.br())),
                                std::min(r1.br().DistanceTo(r2.tl()),
                                         r1.br().DistanceTo(r2.br())));
        }

        return distance;
    }

    double TrackingBlob::Similarity(Finding& other, int max_distance) {
        static const double weight_intersection = 0.4;
        static const double weight_distance = 0.6;

        double similarity = 0;

        auto r1 = this->boundingRect;
        auto area1 = this->boundingRect.area();
        auto r2 = other.GetBoundingRect();
        auto area2 = other.GetBoundingRect().area();
        auto intersection = r1.Intersection(r2);

        // 1. Area - 100% area intersection -> 1
        double smallestArea = std::min(area1, area2);

        // proportion of the areas
        double delta = (double)std::min(area1, area2) / std::max(area1, area2);

        if (intersection.empty()) {  // no intersection
            // 2. Distance
            double probDistance = this->DistanceTo(other);
            probDistance = probDistance > max_distance
                               ? 0
                               : (1.0 - probDistance / max_distance);

            // 3. weight the probs
            similarity =
                delta * weight_intersection + probDistance * weight_distance;

            return similarity;
        } else {  // intersection
            double probIntersection =
                (double)intersection.area() / smallestArea;

            return probIntersection * 0.5 + delta * 0.5;
        }
    }

    Blob TrackingBlob::ToBlob() { return std::move(*this); }

    void TrackingBlob::SetCenter(Rect& rect) {
        this->center = Point(rect.x + rect.width / 2, rect.y + rect.height / 2);
    }
}  // namespace Observer