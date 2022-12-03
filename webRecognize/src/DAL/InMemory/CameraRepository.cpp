#include "CameraRepository.hpp"

#include <spdlog/fmt/bundled/format.h>

#include "Domain/Camera.hpp"

namespace Web::DAL {
    using Domain::Camera;

    std::string CameraRepositoryMemory::Add(Camera& element) {
        element.id = fmt::format("c{0}", cameras.size());
        cameras.push_back(element);
        return element.id;
    }

    void CameraRepositoryMemory::Remove(const std::string& id) {
        auto res = std::find_if(cameras.begin(), cameras.end(),
                                [&id](Camera& cam) { return cam.id == id; });

        if (res != cameras.end()) {
            cameras.erase(res);
        }
    }

    bool CameraRepositoryMemory::Exists(const std::string& id) {
        auto res =
            std::find_if(cameras.begin(), cameras.end(),
                         [&id](const Camera& el) { return el.id == id; });

        return res != cameras.end();
    }

    Camera CameraRepositoryMemory::Get(const std::string& id) {
        auto res =
            std::find_if(cameras.begin(), cameras.end(),
                         [&id](const Camera& el) { return el.id == id; });

        return *res;
    }

    const std::vector<Camera> CameraRepositoryMemory::GetAll(int limit) {
        OBSERVER_ASSERT(limit > 0 && limit <= cameras.size(),
                        "Limit is out of bounds");

        auto max = std::min(limit, (int)cameras.size());

        std::vector<Camera> result;
        result.reserve(max);

        // start at end until rbegin + max.
        // + sign since its reverse iterator.
        result.insert(result.begin(), cameras.rbegin(), cameras.rbegin() + max);

        return result;
    }
}  // namespace Web::DAL