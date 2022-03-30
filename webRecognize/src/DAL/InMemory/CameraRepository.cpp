#include "CameraRepository.hpp"

#include <spdlog/fmt/bundled/format.h>

namespace Web::DAL {
    using Domain::Camera;

    std::string CameraRepositoryMemory::Add(Camera& element) {
        element.id = fmt::format("c{0}", cameras.size());
        cameras.push_back(element);
        return element.id;
    }

    void CameraRepositoryMemory::Remove(const Camera& element) {
        auto res = std::find(cameras.begin(), cameras.end(), element);

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

    const Camera& CameraRepositoryMemory::Get(const std::string& id) {
        auto res =
            std::find_if(cameras.begin(), cameras.end(),
                         [&id](const Camera& el) { return el.id == id; });

        return *res;
    }

    const std::vector<Camera>& CameraRepositoryMemory::GetAll() {
        return cameras;
    }
}  // namespace Web::DAL