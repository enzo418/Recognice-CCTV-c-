#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../Domain/Notification.hpp"
#include "DTO/DTONotificationDebugVideo.hpp"

namespace Web::DAL {

    class INotificationRepository {
       public:
        virtual std::string Add(Domain::Notification& element) = 0;

        virtual void Remove(const std::string& id) = 0;

        virtual bool Exists(const std::string& id) = 0;

        virtual Domain::Notification Get(const std::string& id) = 0;

        /**
         * @brief Get the all the notifications sorted by date, first as the
         * most recent.
         *
         * @param limit
         * @return const std::vector<Domain::Notification>
         */
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

        virtual std::string AddNotificationDebugVideo(
            const Web::DTONotificationDebugVideo&) = 0;

        virtual std::optional<Web::DTONotificationDebugVideo>
        GetNotificationDebugVideo(int groupID) = 0;

        virtual void UpdateNotificationDebugVideo(
            const std::string& id, const std::string& videoBufferID) = 0;
    };
}  // namespace Web::DAL