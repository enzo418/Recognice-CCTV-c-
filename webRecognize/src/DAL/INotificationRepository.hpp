#pragma once

#include <string>
#include <vector>

#include "../Domain/Notification.hpp"

namespace Web::DAL {

    class INotificationRepository {
       public:
        virtual std::string Add(Domain::Notification& element) = 0;

        virtual void Remove(const std::string& id) = 0;

        virtual bool Exists(const std::string& id) = 0;

        virtual Domain::Notification Get(const std::string& id) = 0;

        virtual const std::vector<Domain::Notification> GetAll(
            int limit = 100) = 0;

        /**
         * @brief Get the filename of a notification
         *
         * @param id
         * @return std::string
         */
        virtual std::string GetFilename(const std::string& id) = 0;

        virtual int GetLastGroupID() = 0;
    };
}  // namespace Web::DAL