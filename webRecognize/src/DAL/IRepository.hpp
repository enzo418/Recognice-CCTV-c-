#pragma once

#include <cstdint>
#include <vector>

namespace Web::DAL {
    template <typename T, typename T_ID>
    class IRepository {
       public:
        virtual T_ID Add(T& element) = 0;

        virtual void Remove(const T& element) = 0;

        virtual bool Exists(const T_ID& id) = 0;

        virtual const T& Get(const T_ID& id) = 0;

        virtual const std::vector<T> GetAll(int limit = 100) = 0;
    };
}  // namespace Web::DAL