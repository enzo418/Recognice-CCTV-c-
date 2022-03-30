#pragma once

#include <string>

#include "../Domain/Notification.hpp"
#include "IRepository.hpp"

namespace Web::DAL {
    class INotificationRepository
        : public IRepository<Domain::Notification, std::string> {
       public:
        virtual std::string Add(Domain::Notification& element) = 0;

        virtual void Remove(const Domain::Notification& element) = 0;

        virtual bool Exists(const std::string& id) = 0;

        virtual const Domain::Notification& Get(const std::string& id) = 0;

        virtual const std::vector<Domain::Notification> GetAll(
            int limit = 100) = 0;

        /**
         * @brief Get the filename of a notification
         *
         * @param id
         * @return std::string
         */
        virtual const std::string& GetFilename(const std::string& id) = 0;
    };
}  // namespace Web::DAL