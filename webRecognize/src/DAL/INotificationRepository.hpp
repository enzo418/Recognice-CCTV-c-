#pragma once

#include <ctime>
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
         * @param olderFirst
         * @param page
         * @return const std::vector<Domain::Notification>
         */
        virtual const std::vector<Domain::Notification> GetAll(
            int limit = 100, bool olderFirst = false, int page = 1) = 0;

        /**
         * @brief Get notification between those dates
         *
         * @param start start time in unix time
         * @param end end time in unix time
         * @param limit
         * @param olderFirst
         * @param page
         * @return const std::vector<Domain::Notification>
         */
        virtual const std::vector<Domain::Notification> GetBetweenDates(
            std::time_t start, std::time_t end, int limit = 100,
            bool olderFirst = false, int page = 1) = 0;

        /**
         * @brief Get the filename of a notification
         *
         * @param id
         * @return std::string
         */
        virtual std::string GetFilename(const std::string& id) = 0;

        virtual int GetLastGroupID() = 0;

        // I might later move all of debug video... might

        virtual std::string AddNotificationDebugVideo(
            const Web::DTONotificationDebugVideo&) = 0;

        virtual std::optional<Web::DTONotificationDebugVideo>
        GetNotificationDebugVideo(int groupID) = 0;

        virtual void UpdateNotificationDebugVideo(
            const std::string& id, const std::string& videoBufferID) = 0;

        virtual std::vector<Web::DTONotificationDebugVideo>
        GetNonReclaimedDebugVideos(int limit = 100,
                                   bool olderFirst = false) = 0;

        virtual void RemoveDebugVideoEntry(const std::string& id) = 0;
    };
}  // namespace Web::DAL