#pragma once

#include <string>

namespace Web::Domain {
    class Camera {
       public:
        std::string cameraID;
        std::string name;
        std::string uri;

       public:
        Camera() = default;
        Camera(std::string name, std::string uri);

        auto operator<=>(const Camera&) const = default;
    };
}  // namespace Web::Domain
