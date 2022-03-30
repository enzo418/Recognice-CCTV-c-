#pragma once

#include <algorithm>
#include <iterator>
#include <string>

#include "../../../../recognize/Observer/src/Log/log.hpp"
#include "../ICameraRepository.hpp"

namespace Web::DAL {
    class CameraRepositoryMemory : public ICameraRepository {
       public:
        std::string Add(Domain::Camera& element) override;

        void Remove(const Domain::Camera& element) override;

        bool Exists(const std::string& id) override;

        const Domain::Camera& Get(const std::string& element) override;

        const std::vector<Domain::Camera> GetAll(int limit = 100) override;

       private:
        std::vector<Domain::Camera> cameras;
    };
}  // namespace Web::DAL