#include "NotificationRepositoryNLDB.hpp"

#include <stdexcept>

#include "Serialization/JsonSerialization.hpp"

namespace Web::DAL {
    using Domain::Notification;

    void PrepareNotificationToInsert(nldb::json& n) {
        // remove notificationID, we use the database id as id
        if (n.contains("notificationID")) n.erase("notificationID");

        // remove uri from camera, just store the id and name
        if (n.contains("camera")) n["camera"].erase("uri");
    }

    NotificationRepositoryNLDB::NotificationRepositoryNLDB(nldb::DBSL3* pDb)
        : db(pDb),
          query(db),
          colNotifications("notifications"),
          colNotificationDebugVideo("debugNotificationVideo") {}

    std::string NotificationRepositoryNLDB::Add(Notification& element) {
        nldb::json elementAsJson = element;

        PrepareNotificationToInsert(elementAsJson);

        auto ids = query.from(colNotifications).insert(elementAsJson);
        return ids[0];
    }

    void NotificationRepositoryNLDB::Remove(const std::string& id) {
        query.from(colNotifications).remove(id);
    }

    bool NotificationRepositoryNLDB::Exists(const std::string& id) {
        nldb::json result = query.from(colNotifications)
                                .select(colNotifications["_id"])
                                .where(colNotifications["_id"] == id)
                                .execute();

        return !result.empty();
    }

    Domain::Notification NotificationRepositoryNLDB::Get(
        const std::string& id) {
        nldb::json result =
            query.from(colNotifications)
                .select()
                .where(colNotifications["_id"] == id)
                .rename(colNotifications["_id"], "notificationID")
                .execute();

        if (result.empty()) {
            throw std::runtime_error("Notification not found");
        }

        return result[0];
    }

    const std::vector<Domain::Notification> NotificationRepositoryNLDB::GetAll(
        int limit, bool olderFirst, int page) {
        if (limit < 0) OBSERVER_ERROR("Limit is out of bounds");

        const auto sortProp = olderFirst ? colNotifications["datetime"].asc()
                                         : colNotifications["datetime"].desc();

        nldb::json result =
            query.from(colNotifications)
                .select()
                .sortBy(sortProp)
                .limit(limit)
                .rename(colNotifications["_id"], "notificationID")
                .page(page)
                .execute();

        return result;
    }

    const std::vector<Domain::Notification>
    NotificationRepositoryNLDB::GetBetweenDates(std::time_t start,
                                                std::time_t end, int limit,
                                                bool olderFirst, int page) {
        const auto sortProp = olderFirst ? colNotifications["datetime"].asc()
                                         : colNotifications["datetime"].desc();

        nldb::json result =
            query.from(colNotifications)
                .select()
                .where(colNotifications["datetime"] >= (long)start &&
                       colNotifications["datetime"] <= (long)end)
                .sortBy(sortProp)
                .limit(limit)
                .rename(colNotifications["_id"], "notificationID")
                .page(page)
                .execute();

        return result;
    }

    std::string NotificationRepositoryNLDB::GetFilename(const std::string& id) {
        return this->Get(id).content;
    }

    int NotificationRepositoryNLDB::GetLastGroupID() {
        nldb::json result = query.from(colNotifications)
                                .select(colNotifications["groupID"])
                                .sortBy(colNotifications["groupID"].desc())
                                .limit(1)
                                .execute();

        if (result.empty()) return 0;

        return result[0]["groupID"];
    }

    std::string NotificationRepositoryNLDB::AddNotificationDebugVideo(
        const Web::DTONotificationDebugVideo& element) {
        nldb::json elementAsJson = element;

        elementAsJson.erase("id");

        auto ids = query.from(colNotificationDebugVideo).insert(elementAsJson);

        return ids[0];
    }

    std::optional<Web::DTONotificationDebugVideo>
    NotificationRepositoryNLDB::GetNotificationDebugVideo(int groupID) {
        nldb::json res =
            query.from(colNotificationDebugVideo)
                .select()
                .where(colNotificationDebugVideo["groupID"] == groupID)
                .rename(colNotificationDebugVideo["_id"], "id")
                .execute();

        if (res.size() > 0) {
            return res[0];
        }

        return std::nullopt;
    }

    void NotificationRepositoryNLDB::UpdateNotificationDebugVideo(
        const std::string& id, const std::string& videoBufferID) {
        query.from(colNotificationDebugVideo)
            .update(id, nldb::json {{"videoBufferID", videoBufferID}});
    }

    std::vector<Web::DTONotificationDebugVideo>
    NotificationRepositoryNLDB::GetNonReclaimedDebugVideos(int limit,
                                                           bool olderFirst) {
        const auto sortProp =
            olderFirst ? colNotificationDebugVideo["date_unix"].asc()
                       : colNotificationDebugVideo["date_unix"].desc();

        nldb::json res =
            query.from(colNotificationDebugVideo)
                .select()
                .where(colNotificationDebugVideo["videoBufferID"] == "")
                .sortBy(sortProp)
                .rename(colNotificationDebugVideo["_id"], "id")
                .execute();

        return res;
    }

    void NotificationRepositoryNLDB::RemoveDebugVideoEntry(
        const std::string& id) {
        query.from(colNotificationDebugVideo).remove(id);
    }
}  // namespace Web::DAL