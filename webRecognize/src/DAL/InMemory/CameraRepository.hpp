#pragma once

#include <algorithm>
#include <iterator>
#include <string>

#include "../ICameraRepository.hpp"
#include "observer/Log/log.hpp"

namespace Web::DAL {
    class CameraRepositoryMemory : public ICameraRepository {
       public:
        std::string Add(Domain::Camera& element) override;

        void Remove(const std::string& id) override;

        bool Exists(const std::string& id) override;

        Domain::Camera Get(const std::string& element) override;

        const std::vector<Domain::Camera> GetAll(int limit = 100) override;

       private:
        std::vector<Domain::Camera> cameras;
    };
}  // namespace Web::DAL