#include "NotificationRepository.hpp"

#include <spdlog/fmt/bundled/format.h>

namespace Web::DAL {
    using Domain::Notification;

    std::string NotificationRepositoryMemory::Add(Notification& element) {
        element.id = fmt::format("n{0}", notifications.size());
        notifications.push_back(element);
        return element.id;
    }

    void NotificationRepositoryMemory::Remove(const Notification& element) {
        auto res =
            std::find(notifications.begin(), notifications.end(), element);

        if (res != notifications.end()) {
            notifications.erase(res);
        }
    }

    bool NotificationRepositoryMemory::Exists(const std::string& id) {
        auto res =
            std::find_if(notifications.begin(), notifications.end(),
                         [&id](const Notification& el) { return el.id == id; });

        return res != notifications.end();
    }

    const Notification& NotificationRepositoryMemory::Get(
        const std::string& id) {
        auto res =
            std::find_if(notifications.begin(), notifications.end(),
                         [&id](const Notification& el) { return el.id == id; });

        return *res;
    }

    const std::vector<Notification>& NotificationRepositoryMemory::GetAll() {
        return notifications;
    }

    const std::string& NotificationRepositoryMemory::GetFilename(
        const std::string& id) {
        return this->Get(id).content;
    }
}  // namespace Web::DAL