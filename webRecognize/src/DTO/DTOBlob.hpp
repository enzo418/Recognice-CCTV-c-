#pragma once

#include <iostream>

#include "observer/Blob/BlobDetector/Blob.hpp"

namespace Web {
    struct DTOBlob {
        DTOBlob() = default;

        DTOBlob(const Observer::Blob& blob) {
            this->first_appearance = blob.GetFirstAppearance();
            this->last_appearance = blob.GetLastAppearance();

            for (int i = this->first_appearance; i <= this->last_appearance;
                 i++) {
                this->rects.push_back(blob.GetBoundingRect(i));
            }

            this->internal_id = blob.GetId();
        }

        int first_appearance;
        int last_appearance;
        std::vector<Observer::Rect> rects;
        int internal_id;
    };
}  // namespace Web