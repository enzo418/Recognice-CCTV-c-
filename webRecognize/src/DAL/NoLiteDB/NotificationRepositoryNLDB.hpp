#pragma once

#include <algorithm>
#include <iterator>
#include <string>

#include "../INotificationRepository.hpp"
#include "DTO/DTONotificationDebugVideo.hpp"
#include "Domain/Notification.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Query/Query.hpp"
#include "nldb/SQL3Implementation.hpp"
#include "nldb/backends/sqlite3/DB/DB.hpp"
#include "observer/Log/log.hpp"

namespace Web::DAL {
    class NotificationRepositoryNLDB : public INotificationRepository {
       public:
        NotificationRepositoryNLDB(nldb::DBSL3* db);

       public:
        std::string Add(Domain::Notification& element) override;

        void Remove(const std::string& id) override;

        bool Exists(const std::string& id) override;

        Domain::Notification Get(const std::string& id) override;

        const std::vector<Domain::Notification> GetAll(
            int limit = 100) override;

        std::string GetFilename(const std::string& id) override;

        /**
         * @brief Get the last group id stored, defaults to 0.
         *
         * @return int last group id or 0
         */
        int GetLastGroupID() override;

        std::string AddNotificationDebugVideo(
            const Web::DTONotificationDebugVideo&) override;

        std::optional<Web::DTONotificationDebugVideo> GetNotificationDebugVideo(
            int groupID) override;

        void UpdateNotificationDebugVideo(
            const std::string& id, const std::string& videoBufferID) override;

       private:
        nldb::DBSL3* db;

        nldb::Query<nldb::DBSL3> query;

        nldb::Collection colNotifications;
        nldb::Collection colNotificationDebugVideo;
    };
}  // namespace Web::DAL