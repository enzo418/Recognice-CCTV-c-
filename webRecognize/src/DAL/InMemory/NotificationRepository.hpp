#pragma once

#include <algorithm>
#include <iterator>
#include <string>

#include "../../../../recognize/Observer/src/Log/log.hpp"
#include "../INotificationRepository.hpp"

namespace Web::DAL {
    class NotificationRepositoryMemory : public INotificationRepository {
       public:
        std::string Add(Domain::Notification& element) override;

        void Remove(const Domain::Notification& element) override;

        bool Exists(const std::string& id) override;

        const Domain::Notification& Get(const std::string& element) override;

        const std::vector<Domain::Notification> GetAll(
            int limit = 100) override;

        const std::string& GetFilename(const std::string& id) override;

       private:
        std::vector<Domain::Notification> notifications;
    };
}  // namespace Web::DAL