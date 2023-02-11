#pragma once

#include <algorithm>
#include <iterator>
#include <string>

#include "../INotificationRepository.hpp"
#include "Domain/Notification.hpp"
#include "observer/Log/log.hpp"

namespace Web::DAL {

    class NotificationRepositoryMemory : public INotificationRepository {
       public:
        std::string Add(Domain::Notification& element) override;

        void Remove(const std::string& id) override;

        bool Exists(const std::string& id) override;

        Domain::Notification Get(const std::string& element) override;

        const std::vector<Domain::Notification> GetAll(int limit = 100,
                                                       bool olderFirst = false,
                                                       int page = 1) override;

        std::string GetFilename(const std::string& id) override;

        int GetLastGroupID() override;

       private:
        std::vector<Domain::Notification> notifications;
    };
}  // namespace Web::DAL