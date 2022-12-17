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
        : db(pDb), query(db), colNotifications("notifications") {}

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
        nldb::json result = query.from(colNotifications)
                                .select()
                                .where(colNotifications["_id"] == id)
                                .execute();

        if (result.empty()) {
            throw std::runtime_error("Notification not found");
        }

        for (auto& r : result) {
            r["notificationID"] = r["_id"];
        }

        return result[0];
    }

    const std::vector<Domain::Notification> NotificationRepositoryNLDB::GetAll(
        int limit) {
        if (limit < 0) OBSERVER_ERROR("Limit is out of bounds");

        nldb::json result =
            query.from(colNotifications).select().limit(limit).execute();

        for (auto& r : result) {
            r["notificationID"] = r["_id"];
        }

        std::cout << "result: " << result << std::endl;

        return result;
    }

    std::string NotificationRepositoryNLDB::GetFilename(const std::string& id) {
        return this->Get(id).content;
    }
}  // namespace Web::DAL