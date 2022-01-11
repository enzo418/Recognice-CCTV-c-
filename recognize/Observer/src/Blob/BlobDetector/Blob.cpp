#include "Blob.hpp"

namespace Observer {
    Blob::Blob() { this->id = -1; }

    Blob::Blob(int pId) { this->id = pId; }

    int Blob::GetId() { return this->id; }

    int Blob::GetFirstAppearance() {
        return this->appearsOnFrames.empty() ? 0
                                             : this->appearsOnFrames.front();
    }
    int Blob::GetLastAppearance() {
        return this->appearsOnFrames.empty() ? 0 : this->appearsOnFrames.back();
    }

    Vec Blob::GetVelocity(int frameIndex) {
        if (this->velocities.find(frameIndex) != this->velocities.end()) {
            return this->velocities.at(frameIndex);
        } else {
            return {};
        }
    }

    Rect Blob::GetBoundingRect(int frameIndex) {
        if (this->boundingRects.find(frameIndex) != this->boundingRects.end()) {
            return this->boundingRects.at(frameIndex);
        } else {
            return {};
        }
    }

    Point Blob::GetCenter(int frameIndex) {
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
}  // namespace Observer