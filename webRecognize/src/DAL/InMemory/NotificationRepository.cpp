#include "NotificationRepository.hpp"

#include <spdlog/fmt/bundled/format.h>

#include "Domain/Notification.hpp"

namespace Web::DAL {
    using Domain::Notification;

    std::string NotificationRepositoryMemory::Add(Notification& element) {
        element.id = fmt::format("n{0}", notifications.size());
        notifications.push_back(element);
        return element.id;
    }

    void NotificationRepositoryMemory::Remove(const std::string& id) {
        auto res = std::find_if(notifications.begin(), notifications.end(),
                                [&id](Notification& n) { return n.id == id; });

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

    Notification NotificationRepositoryMemory::Get(const std::string& id) {
        auto res =
            std::find_if(notifications.begin(), notifications.end(),
                         [&id](const Notification& el) { return el.id == id; });

        return *res;
    }

    const std::vector<Notification> NotificationRepositoryMemory::GetAll(
        int limit) {
        if (limit < 0) OBSERVER_ERROR("Limit is out of bounds");

        auto max = std::min(limit, (int)notifications.size());

        std::vector<Notification> result;
        result.reserve(max);

        // start at end until rbegin + max.
        // + sign since its reverse iterator.
        result.insert(result.begin(), notifications.rbegin(),
                      notifications.rbegin() + max);

        return result;
    }

    std::string NotificationRepositoryMemory::GetFilename(
        const std::string& id) {
        return this->Get(id).content;
    }
}  // namespace Web::DAL