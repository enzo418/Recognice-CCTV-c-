#pragma once

#include <memory>

#include "../DAL/INotificationRepository.hpp"
#include "../Domain/Notification.hpp"
#include "lrucache11/LRUCache11.hpp"

namespace Web::CL {
    class NotificationCL {
       public:
        NotificationCL(DAL::INotificationRepository* repo);

       public:
        /**
         * @brief Get a notification by id.
         *
         * @param id
         * @return Domain::Notification A copy of the notification, since
         * it cannot return a reference since it might be deleted right after
         * the call.
         */
        Domain::Notification Get(const std::string& id);

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

        /**
         * @brief Get the filename of a notification by id.
         *
         * @param id
         * @return std::string returns a copy of the filename.
         */
        std::string GetFilename(const std::string& id);

       private:
        DAL::INotificationRepository* notificationRepository;

       private:
        lru11::Cache<std::string, Domain::Notification> notificationCache;
        lru11::Cache<std::string, std::string> notificationFilenameCache;
    };
}  // namespace Web::CL