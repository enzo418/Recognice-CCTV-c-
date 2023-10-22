#include "Camera.hpp"

namespace Web::Domain {
    Camera::Camera(const std::string& pName, const std::string& pUri)
        : name(std::move(pName)), uri(pUri) {}
}  // namespace Web::Domain