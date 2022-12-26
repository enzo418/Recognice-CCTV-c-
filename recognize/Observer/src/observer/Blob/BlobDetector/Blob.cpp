#include "Blob.hpp"

namespace Observer {
    Blob::Blob() { this->id = -1; }

    Blob::Blob(int pId) { this->id = pId; }

    int Blob::GetId() const { return this->id; }

    int Blob::GetFirstAppearance() const {
        return this->appearsOnFrames.empty() ? 0
                                             : this->appearsOnFrames.front();
    }
    int Blob::GetLastAppearance() const {
        return this->appearsOnFrames.empty() ? 0 : this->appearsOnFrames.back();
    }

    Vec Blob::GetVelocity(int frameIndex) const {
        if (this->velocities.find(frameIndex) != this->velocities.end()) {
            return this->velocities.at(frameIndex);
        } else {
            return {};
        }
    }

    Rect Blob::GetBoundingRect(int frameIndex) const {
        if (this->boundingRects.find(frameIndex) != this->boundingRects.end()) {
            return this->boundingRects.at(frameIndex);
        } else {
            return {};
        }
    }

    Point Blob::GetCenter(int frameIndex) const {
        if (this->centers.find(frameIndex) != this->centers.end()) {
            return this->centers.at(frameIndex);
        } else {
            return {};
        }
    }

    std::vector<int>& Blob::GetAppearances() { return this->appearsOnFrames; }

    double Blob::GetAverageMagnitude() {
        if (velocities.empty()) return 0;

        double total = 0;

        for (auto& vel : velocities) {
            total += vel.second.magnitude();
        }

        return total / velocities.size();
    }

    double Blob::GetDistanceTraveled() {
        if (appearsOnFrames.size() < 2) return 0;

        double distance = 0;
        Point lastPoint = centers[appearsOnFrames[0]];
        for (size_t i = 1; i < appearsOnFrames.size(); i++) {
            Point center = centers[appearsOnFrames[i]];
            distance += lastPoint.DistanceTo(center);
            lastPoint = center;
        }

        return distance / appearsOnFrames.size();
    }
}  // namespace Observer