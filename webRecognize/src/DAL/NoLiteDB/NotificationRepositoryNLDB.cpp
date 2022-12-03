#include "NotificationRepositoryNLDB.hpp"

#include <stdexcept>

#include "Serialization/JsonSerialization.hpp"

namespace Web::DAL {
    using Domain::Notification;

    NotificationRepositoryNLDB::NotificationRepositoryNLDB(nldb::DBSL3* pDb)
        : db(pDb), query(db), colNotifications("notifications") {}

    std::string NotificationRepositoryNLDB::Add(Notification& element) {
        nldb::json elementAsJson = element;
        auto ids = query.from(colNotifications).insert(elementAsJson);
        element.id = ids[0];
        return element.id;
    }

    void NotificationRepositoryNLDB::Remove(const std::string& id) {
        query.from(colNotifications).remove(id);
    }

    bool NotificationRepositoryNLDB::Exists(const std::string& id) {
        nldb::json result = query.from(colNotifications)
                                .select(colNotifications["id"])
                                .where(colNotifications["id"] == id)
                                .execute();

        return !result.empty();
    }

    Notification NotificationRepositoryNLDB::Get(const std::string& id) {
        nldb::json result = query.from(colNotifications)
                                .select()
                                .where(colNotifications["id"] == id)
                                .includeInnerIds()
                                .execute();

        if (result.empty()) {
            throw std::runtime_error("Notification not found");
        }

        return result[0];
    }

    const std::vector<Notification> NotificationRepositoryNLDB::GetAll(
        int limit) {
        if (limit < 0) OBSERVER_ERROR("Limit is out of bounds");

        nldb::json result =
            query.from(colNotifications).select().limit(limit).execute();

        std::cout << "result: " << result << std::endl;

        return result;
    }

    std::string NotificationRepositoryNLDB::GetFilename(const std::string& id) {
        return this->Get(id).content;
    }
}  // namespace Web::DAL