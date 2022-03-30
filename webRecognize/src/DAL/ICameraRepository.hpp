#pragma once

#include <string>

#include "../Domain/Camera.hpp"
#include "IRepository.hpp"

namespace Web::DAL {
    class ICameraRepository : public IRepository<Domain::Camera, std::string> {
       public:
        virtual std::string Add(Domain::Camera& element) = 0;

        virtual void Remove(const Domain::Camera& element) = 0;

        virtual bool Exists(const std::string& id) = 0;

        virtual const Domain::Camera& Get(const std::string& id) = 0;

        virtual const std::vector<Domain::Camera> GetAll(int limit = 100) = 0;
    };
}  // namespace Web::DAL