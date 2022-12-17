#pragma once

#include <string>
#include <vector>

#include "../Domain/Camera.hpp"

namespace Web::DAL {

    class ICameraRepository {
       public:
        virtual std::string Add(Domain::Camera& element) = 0;

        virtual void Remove(const std::string& id) = 0;

        virtual bool Exists(const std::string& id) = 0;

        virtual Domain::Camera Get(const std::string& id) = 0;

        virtual const std::vector<Domain::Camera> GetAll(int limit = 100) = 0;
    };
}  // namespace Web::DAL