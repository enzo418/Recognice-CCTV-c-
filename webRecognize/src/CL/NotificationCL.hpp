#pragma once

#include <memory>

#include "../../vendor/LRUCache11.hpp"
#include "../DAL/INotificationRepository.hpp"
#include "../Domain/Notification.hpp"

namespace Web::CL {
    class NotificationCL {
       public:
        NotificationCL(DAL::INotificationRepository* repo);

       public:
        const Domain::Notification& Get(const std::string& id);

        /**
         * @brief Checks if a notification exists.
         * First it checks on cached notifications and if it cannot be found it
         * checks on the repository.
         *
         * @param id
         * @return true
         * @return false
         */
        bool Exists(const std::string& id);

        const std::string& GetFilename(const std::string& id);

       private:
        DAL::INotificationRepository* notificationRepository;

       private:
        lru11::Cache<std::string, Domain::Notification> notificationCache;
        lru11::Cache<std::string, std::string> notificationFilenameCache;
    };
}  // namespace Web::CL